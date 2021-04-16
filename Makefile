EXECNAME=SLDM

SRC_DIR=src
OBJ_DIR=obj

SRC=$(wildcard $(SRC_DIR)/*.c)

OBJ=$(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

OBJ_CC=$(OBJ)

CFLAGS += -Wall -O3 -Iinclude
LDLIBS += -lcpprest -lpthread -lcrypto -lm

.PHONY: all clean

all: compilePC

compilePC: CC = gcc
	
compilePCdebug: CFLAGS += -g
compilePCdebug: compilePC

compilePC compilePCdebug: $(EXECNAME)

# Standard targets
$(EXECNAME): $(OBJ_CC)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@ mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ_DIR)/*.o
	-rm -rf $(OBJ_DIR)
	
fullclean: clean
	$(RM) $(EXECNAME)