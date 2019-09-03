
CC = gcc
TARGET = shell
SRCS = shell.c shell_main.c
INC = -I../client_info_inc  
OBJS = shell.o shell_main.o
CFLAGS = -Wall -Wextra
LIBS = -lm -lmosquitto

all: $(TARGET)

$(TARGET): $(OBJS) ../client_info_inc/client_info.h
	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS) $(LIBS) $(INC)

shell_main.o: shell_main.c ../client_info_inc/client_info.h 
	     $(CC) -c shell_main.c $(CFLAGS) $(INC)

shell.o: shell.c shell.h ../client_info_inc/client_info.h
	$(CC) -c shell.c $(CFLAGS) $(INC)

.PHONY: clean
clean:
	rm $(OBJS)






