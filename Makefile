EXECNAME=SLDM

SRC_DIR=src
OBJ_DIR=obj

SRC_VEHVIS_DIR=vehicle-visualizer/src
OBJ_VEHVIS_DIR=obj/vehicle-visualizer

SRC_OPTIONS_DIR=options
OBJ_OPTIONS_DIR=obj/options

SRC_DECODER_DIR=decoder-module/src
OBJ_OPTIONS_DIR=obj/decoder-module

SRC_ASN1_DIR=decoder-module/asn1/src
OBJ_ASN1_DIR=obj/asn1

SRC=$(wildcard $(SRC_DIR)/*.cpp)
SRC_VEHVIS=$(wildcard $(SRC_VEHVIS_DIR)/*.cc)
SRC_OPTIONS=$(wildcard $(SRC_OPTIONS_DIR)/*.c)
SRC_DECODER=$(wildcard $(SRC_DECODER_DIR)/*.cpp)
SRC_ASN1=$(wildcard $(SRC_ASN1_DIR)/*.c)

OBJ=$(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
OBJ_VEHVIS=$(SRC_VEHVIS:$(SRC_VEHVIS_DIR)/%.c=$(OBJ_VEHVIS_DIR)/%.o)
OBJ_OPTIONS=$(SRC_OPTIONS:$(SRC_OPTIONS_DIR)/%.c=$(OBJ_OPTIONS_DIR)/%.o)
OBJ_DECODER=$(SRC_DECODER:$(SRC_DECODER_DIR)/%.c=$(OBJ_DECODER_DIR)/%.o)
OBJ_ASN1=$(SRC_ASN1:$(SRC_ASN1_DIR)/%.c=$(OBJ_ASN1_DIR)/%.o)

OBJ_CC=$(OBJ)
OBJ_CC+=$(OBJ_VEHVIS)
OBJ_CC+=$(OBJ_OPTIONS)
OBJ_CC+=$(OBJ_DECODER)
OBJ_CC+=$(OBJ_ASN1)

CXXFLAGS += -Wall -O3 -Iinclude -Ivehicle-visualizer/include -Ioptions -std=c++17 -Idecoder-module/include -Idecoder-module/asn1/include
CFLAGS += -Wall -O3 -Iinclude -Ioptions -Idecoder-module/asn1/include
LDLIBS += -lcpprest -lpthread -lcrypto -lm -lqpid-proton-cpp -lGeographic

.PHONY: all clean

all: compilePC

compilePC: CXX = g++
compilePC: CC = gcc
	
compilePCdebug: CXXFLAGS += -g
compilePCdebug: compilePC

compilePC compilePCdebug: $(EXECNAME)

# Standard targets
$(EXECNAME): $(OBJ_CC)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) $(CXXFLAGS) $(CFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@ mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_VEHVIS_DIR)/%.o: $(SRC_VEHVIS_DIR)/%.cc
	@ mkdir -p $(OBJ_VEHVIS_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_OPTIONS_DIR)/%.o: $(SRC_OPTIONS_DIR)/%.c
	@ mkdir -p $(OBJ_OPTIONS_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DECODER_DIR)/%.o: $(SRC_DECODER_DIR)/%.cpp
	@ mkdir -p $(OBJ_DECODER_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_ASN1_DIR)/%.o: $(SRC_ASN1_DIR)/%.c
	@ mkdir -p $(OBJ_ASN1_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ_DIR)/*.o $(OBJ_VEHVIS_DIR)/*.o $(OBJ_OPTIONS_DIR)/*.o$(OBJ_DECODER_DIR)/*.o $(OBJ_ASN1_DIR)/*.o
	-rm -rf $(OBJ_DIR)
	-rm -rf $(OBJ_VEHVIS_DIR)
	-rm -rf $(OBJ_OPTIONS_DIR)
	-rm -rf $(OBJ_DECODER_DIR)
	-rm -rf $(OBJ_ASN1_DIR)
	-rm -f cachefile.sldmc
	
fullclean: clean
	$(RM) $(EXECNAME)
