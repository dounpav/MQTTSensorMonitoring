
CC = gcc
TARGET = shell_client
SRCS = shell_client.c shell_client_main.c
INC = -I../client_info_inc  
OBJS = shell_client.o shell_client_main.o #$(SRCS:.o)
CFLAGS = -Wall -Wextra
LIBS = -lmosquitto

all: $(TARGET)

$(TARGET): $(OBJS) ../client_info_inc/client_info.h
	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS) $(LIBS) $(INC)

shell_client_main.o: shell_client_main.c ../client_info_inc/client_info.h 
	     $(CC) -c shell_client_main.c $(CFLAGS) $(INC)

shell_client.o: shell_client.c shell_client.h ../client_info_inc/client_info.h
	$(CC) -c shell_client.c $(CFLAGS) $(INC)

.PHONY: clean
clean:
	rm $(OBJS)






