CFLAGS = -Wall -Wextra -lpthread

all: memmap
%: %.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) memmap
