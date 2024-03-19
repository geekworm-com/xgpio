#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define SHUTDOWN_LINE 4
#define BOOT_LINE 17
#define REBOOT_PULSE_MINIMUM 200
#define REBOOT_PULSE_MAXIMUM 600

int main() {
    struct gpiod_chip *chip;
    struct gpiod_line *shutdown_line, *boot_line;
    int ret;
    long pulse_duration;

    // Open GPIO chip
    chip = gpiod_chip_open("gpiochip0");
    if (!chip) {
        perror("Open GPIO chip failed");
        return EXIT_FAILURE;
    }

    // Get GPIO line handles
    shutdown_line = gpiod_chip_get_line(chip, SHUTDOWN_LINE);
    if (!shutdown_line) {
        perror("Get shutdown GPIO line failed");
        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    boot_line = gpiod_chip_get_line(chip, BOOT_LINE);
    if (!boot_line) {
        perror("Get boot GPIO line failed");
        gpiod_line_release(shutdown_line);
        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    // Configure GPIO lines
    ret = gpiod_line_request_input(shutdown_line, "shutdown");
    if (ret < 0) {
        perror("Configure shutdown GPIO line as input failed");
        gpiod_line_release(boot_line);
        gpiod_line_release(shutdown_line);
        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    ret = gpiod_line_request_output(boot_line, "boot", 1);
    if (ret < 0) {
        perror("Configure boot GPIO line as output failed");
        gpiod_line_release(boot_line);
        gpiod_line_release(shutdown_line);
        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    printf("Your device is ready.\n");

    while (true) {
        // Read shutdown GPIO line value
        int shutdown_signal = gpiod_line_get_value(shutdown_line);
        if (shutdown_signal < 0) {
            perror("Read shutdown GPIO line value failed");
            break;
        }

        if (shutdown_signal == 0) {
            usleep(200000); // 200ms
        } else {
            struct timespec pulse_start, current_time;
            clock_gettime(CLOCK_MONOTONIC, &pulse_start);

            while (shutdown_signal == 1) {
                usleep(20000); // 20ms

                clock_gettime(CLOCK_MONOTONIC, &current_time);
                pulse_duration = (current_time.tv_sec - pulse_start.tv_sec) * 1000 +
                                      (current_time.tv_nsec - pulse_start.tv_nsec) / 1000000;

                if (pulse_duration > REBOOT_PULSE_MAXIMUM) {
                    printf("Your device is shutting down, halting Rpi...\n");
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
                printf("Your device is rebooting, recycling Rpi...\n");
                system("sudo reboot");
                break;
            }
        }
    }

    // Release GPIO line resources
    gpiod_line_release(boot_line);
    gpiod_line_release(shutdown_line);
    gpiod_chip_close(chip);

    return EXIT_SUCCESS;
}


// How to compile this file;
// gcc xgpio-pwr.c -o xgpio_pwr -lgpiod
// xgpio-pwr gpiochip0 4 17

//  view gpio map
// cat /sys/kernel/debug/gpio