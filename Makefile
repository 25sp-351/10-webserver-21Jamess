CC      = gcc
CFLAGS  = -D_POSIX_C_SOURCE=200809L -Wall -Wextra -pthread -std=c11
SRCS    = main.c server.c http.c handler.c
OBJS    = $(SRCS:.c=.o)
TARGET  = webserver

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)
