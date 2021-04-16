EXECNAME=SLDM

SRC_DIR=src
OBJ_DIR=obj

SRC_VEHVIS_DIR=vehicle-visualizer/src
OBJ_VEHVIS_DIR=obj/vehicle-visualizer

SRC=$(wildcard $(SRC_DIR)/*.c)
SRC_VEHVIS=$(wildcard $(SRC_VEHVIS_DIR)/*.c)

OBJ=$(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
OBJ_VEHVIS=$(SRC_VEHVIS:$(SRC_VEHVIS_DIR)/%.c=$(OBJ_VEHVIS_DIR)/%.o)

OBJ_CC=$(OBJ)
OBJ_CC+=$(OBJ_VEHVIS)

CFLAGS += -Wall -O3 -Iinclude -Ivehicle-visualizer/include
LDLIBS += -lcpprest -lpthread -lcrypto -lm

.PHONY: all clean

all: compilePC

compilePC: CXX = g++
compilePC: CC = gcc
	
compilePCdebug: CFLAGS += -g
compilePCdebug: compilePC

compilePC compilePCdebug: $(EXECNAME)

# Standard targets
$(EXECNAME): $(OBJ_CC)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@ mkdir -p $(OBJ_DIR)
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJ_VEHVIS_DIR)/%.o: $(SRC_VEHVIS_DIR)/%.c
	@ mkdir -p $(OBJ_VEHVIS_DIR)
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ_DIR)/*.o $(OBJ_VEHVIS_DIR)/*.o
	-rm -rf $(OBJ_DIR)
	-rm -rf $(OBJ_VEHVIS_DIR)
	
fullclean: clean
	$(RM) $(EXECNAME)