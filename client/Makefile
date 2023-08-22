CC=cc
SRCDIR=src
OBJDIR=obj
BIN=remote-fb-client
FLAGS=-Wall -lraylib

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))


all: $(BIN)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(FLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(CC) $(FLAGS) -o $@ $^

clean:
	rm -rf $(OBJDIR)

.PHONY: clean
