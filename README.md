# 从内核6.2开始，从Ubuntu 23.04版本开始，sysfs访问GPIO被取消了，无法使用echo 27 > /sys/class/gpio/export 这种方式来访问GPIO，官方推荐使用libgpiod。
# 这个项目仅仅服务于Geekworm的系列产品，象X728, NASPi系列， X735, X715等；使用了libgpiod库来实现在用户空间对GPIO的访问和控制；
# 如果要编译，需要安装libgpiod-dev，可以运行下面的命令安装相应的环境：
# sudo apt install -y libgpiod-dev gcc git

# 编译命令如下：
# gcc xgpio_pwr.c -o xgpio_pwr -lgpiod
# gcc xgpio_soft.c -o xgpio_soft -lgpiod

# 如何使用这两个程序，这两个程序是用来替换x-c1-pwr.sh 和 x-c1-softsd.sh, GPIOCHIP编号和GPIO offset从命令行输入；
# for NASPi series such NASPi，NASPi Gemini 2.5,NASPi Gemini 3.5, NASPi CM4-M2, NASPi CM4-2.5
# xgpio_pwr gpiochip0 4 17
# xgpio_soft gpiochip0 27

# 查看内核中GPIO的映射关系：
# cat /sys/kernel/debug/gpio
