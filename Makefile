CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

SRC = src/main.c src/parse.c src/prompt.c src/hop.c src/reveal.c src/execute.c src/log.c src/pipe.c src/jobs.c src/activities.c src/ping.c src/signals.c src/job_control.c
OBJ = $(SRC:.c=.o)

TARGET = shell.out

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all
