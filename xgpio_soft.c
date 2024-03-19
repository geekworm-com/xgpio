#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <gpiod.h>

#ifndef	CONSUMER
#define	CONSUMER	"XGPIO-SOFT"
#endif

int main(int argc, char *argv[]) {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    unsigned int gpio_line_offset;
    unsigned int delay_seconds = 4; // 默认值

    // 检查命令行参数数量
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <gpio-chip-path> <gpio-line-offset> [<delay-seconds>]\n", argv[0]);
        fprintf(stderr, "\tgpio-chip-path can generally be passed gpiochip0 \n");
        return EXIT_FAILURE;
    }

    // 获取GPIO芯片路径
    const char *chip_name = argv[1];

    // 获取GPIO线偏移量
    if (sscanf(argv[2], "%u", &gpio_line_offset) != 1) {
        fprintf(stderr, "Invalid GPIO line offset: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    // 如果提供了延迟秒数，则获取它
    if (argc == 4) {
        if (sscanf(argv[3], "%u", &delay_seconds) != 1) {
            fprintf(stderr, "Invalid delay seconds: %s\n", argv[3]);
            return EXIT_FAILURE;
        }
    }

    // 打开GPIO芯片
    chip = gpiod_chip_open_by_name(chip_name);
    if (!chip) {
        // perror("gpiod_chip_open_by_name");
        printf("Open chip by name failed. name: %s\n", chip_name);
        goto end;
    }

    // 请求GPIO线
    line = gpiod_chip_get_line(chip, gpio_line_offset);
    if (!line) {
        // perror("gpiod_chip_get_line");
        printf("Get line failed. line_num: %u\n", gpio_line_offset);
        goto close_chip;
    }

    // 设置GPIO线为输出方向
    if (gpiod_line_request_output(line, CONSUMER, 0) < 0) {
        // perror("gpiod_line_request_output");
        printf("Request line as output failed\n");
        goto release_line;
    }

    // 设置GPIO线的初始值
    if (gpiod_line_set_value(line, 1) < 0) {
        // perror("gpiod_line_set_value");
        printf("Set value to 1 at line failed\n");
        goto release_line;
    }

    // 输出提示信息
    printf("Your device will be shutting down in %u seconds...\n", delay_seconds);

    // 等待指定的秒数
    sleep(delay_seconds);

    // 设置GPIO线的最终值
    if (gpiod_line_set_value(line, 0) < 0) {
        // perror("gpiod_line_set_value");
        printf("Set value to 0 at line failed\n");
        goto release_line;
    }
release_line:
    // 释放GPIO线
    gpiod_line_release(line);

close_chip:
    gpiod_chip_close(chip);

end:
    return EXIT_SUCCESS;
}

// How to compile this file;
// gcc xgpio-soft.c -o xgpio-soft -lgpiod
// xgpio-soft gpiochip0 20

//  view gpio map
// cat /sys/kernel/debug/gpio