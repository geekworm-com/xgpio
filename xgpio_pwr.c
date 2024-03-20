#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* ---------------------------------------
1. How to compile this program
   gcc xgpio_pwr.c -o xgpio_pwr -lgpiod

 2. How to use this program
   xgpio_pwr gpiochip0 4 17   
--*/


#define REBOOT_PULSE_MINIMUM 200
#define REBOOT_PULSE_MAXIMUM 600

#ifndef	CONSUMER
#define	CONSUMER	"XGPIO-PWR"
#endif


char *seconds_to_datetime(time_t seconds) {
    static char buffer[20]; // Static character array used to store the result, ensuring access even after function call ends

    struct tm *timeinfo = localtime(&seconds); // Convert seconds to local time structure
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo); // Format the time string
    return buffer;
}


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <CHIP_NAME> <SHUTDOWN_LINE> <BOOT_LINE>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *chip_name = argv[1];

    unsigned int shutdown_line_offset;  // = atoi(argv[2]);
    unsigned int boot_line_offset;      // = atoi(argv[3]);

     // 获取GPIO线偏移量
    if (sscanf(argv[2], "%u", &shutdown_line_offset) != 1) {
        fprintf(stderr, "Invalid GPIO line offset: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    // 获取GPIO线偏移量
    if (sscanf(argv[3], "%u", &boot_line_offset) != 1) {
        fprintf(stderr, "Invalid GPIO line offset: %s\n", argv[3]);
        return EXIT_FAILURE;
    }
  
    struct gpiod_chip *chip;
    struct gpiod_line *shutdown_line, *boot_line;
    int ret;
    long pulse_duration;

    chip = gpiod_chip_open_by_name(chip_name);
    if (!chip) {
        perror("Open GPIO chip by name failed");
        return EXIT_FAILURE;
    }

    shutdown_line = gpiod_chip_get_line(chip, shutdown_line_offset);
    if (!shutdown_line) {
        perror("Get shutdown GPIO line failed");
        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    boot_line = gpiod_chip_get_line(chip, boot_line_offset);
    if (!boot_line) {
        perror("Get boot GPIO line failed");
        gpiod_line_release(shutdown_line);
        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    ret = gpiod_line_request_input(shutdown_line, CONSUMER);
    if (ret < 0) {
        perror("Configure shutdown GPIO line as input failed");
        gpiod_line_release(boot_line);
        gpiod_line_release(shutdown_line);
        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    ret = gpiod_line_request_output(boot_line, CONSUMER, 1);
    if (ret < 0) {
        perror("Configure boot GPIO line as output failed");
        gpiod_line_release(boot_line);
        gpiod_line_release(shutdown_line);
        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    printf("Your device is ready.\n");

    while (true) {
        int shutdown_signal = gpiod_line_get_value(shutdown_line);
        if (shutdown_signal < 0) {
            perror("Read shutdown GPIO line value failed");
            break;
        }

        if (shutdown_signal == 0) {
            usleep(200000);
        } else {
            struct timespec pulse_start, current_time;
            clock_gettime(CLOCK_MONOTONIC, &pulse_start);

            while (shutdown_signal == 1) {
                usleep(20000);

                clock_gettime(CLOCK_MONOTONIC, &current_time);
                pulse_duration = (current_time.tv_sec - pulse_start.tv_sec) * 1000 +
                                      (current_time.tv_nsec - pulse_start.tv_nsec) / 1000000;
                printf("pulse_duration is %ld\n", pulse_duration);

                if (pulse_duration > REBOOT_PULSE_MAXIMUM) {
                    printf("%s: Your device is shutting down, halting Rpi...\n", seconds_to_datetime(time(NULL)) );
                    sync();
                    system("sudo poweroff");
                    break;
                }

                shutdown_signal = gpiod_line_get_value(shutdown_line);
                if (shutdown_signal < 0) {
                    perror("Read shutdown GPIO line value failed");
                    break;
                }
            }

            if (pulse_duration > REBOOT_PULSE_MINIMUM && pulse_duration <= REBOOT_PULSE_MAXIMUM) {
                printf("%s: Your device is rebooting, recycling Rpi...\n", seconds_to_datetime(time(NULL)) );
                sync();
                system("sudo reboot");
                break;
            }
        }
    }

    gpiod_line_release(boot_line);
    gpiod_line_release(shutdown_line);
    gpiod_chip_close(chip);

    return EXIT_SUCCESS;
}
