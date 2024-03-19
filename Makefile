# 定义编译器和编译选项  
CC = gcc  
CFLAGS = -Wall -g -Wextra -std=c11
  
# 定义目标文件和依赖关系  
xgpio_pwr: xgpio_pwr.o  
	$(CC) $(CFLAGS) -o xgpio_pwr xgpio_pwr.o  
  
xgpio_soft: xgpio_soft.o  
	$(CC) $(CFLAGS) -o xgpio_soft xgpio_soft.o  
  
# 定义.o文件的依赖关系和生成规则  
xgpio_pwr.o: xgpio_pwr.c  
	$(CC) $(CFLAGS) -c xgpio_pwr.c  
  
xgpio_soft.o: xgpio_soft.c  
	$(CC) $(CFLAGS) -c xgpio_soft.c  
  
# 定义伪目标clean，用于清理生成的文件  
.PHONY: clean  
clean:  
	rm -f *.o xgpio_pwr xgpio_soft