# Compiler settings - Can change cc to gcc or clang if preferred
CC=gcc
CFLAGS=-Wall

# Build targets
all: TCP_Receiver TCP_Sender

TCP_Receiver: TCP_Receiver.c
	$(CC) $(CFLAGS) -o TCP_Receiver TCP_Receiver.c

TCP_Sender: TCP_Sender.c
	$(CC) $(CFLAGS) -o TCP_Sender TCP_Sender.c

# Custom command to run TCP_Receiver with arguments
run_receiver:
	./TCP_Receiver -p 12345 -algo cubic

# Custom command to run TCP_Sender with arguments
run_sender:
	./TCP_Sender -ip 192.168.37.128 -p 12345 -algo cubic

# Clean the built binaries
clean:
	rm -f TCP_Receiver TCP_Sender

