# This project is only for Geekworm Products such as NASPi, X735, X728 etc.


# How to compile this program;
# gcc xgpio_pwr.c -o xgpio_pwr -lgpiod
# gcc xgpio_soft.c -o xgpio_soft -lgpiod

# How to user this program:
# 对于NASPi系列：
# xgpio_pwr gpiochip0 4 17
# xgpio_soft gpiochip0 27

#  view gpio map
# cat /sys/kernel/debug/gpio
