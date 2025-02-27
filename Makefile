CC=g++
CFLAGS=-Wall
LDFLAGS= -lgtest -lgmock -lgtest_main
BUILD_DIR=build
PROGRAM_EXE=test


all: $(BUILD_DIR) $(BUILD_DIR)/$(PROGRAM_EXE)
	@echo "================ Targets ================"
	@echo "==> clean: to clean the project"
	@echo "==> run: to build and run the program"
	@echo "========================================="

$(BUILD_DIR)/$(PROGRAM_EXE): $(BUILD_DIR)/test.o
	$(CC) $+ $(LDFLAGS) -o $@

$(BUILD_DIR)/test.o: test.cpp queue.h
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: all clean run

$(BUILD_DIR):
	mkdir -p $@


clean: 
	@rm -rf $(BUILD_DIR)

run: $(BUILD_DIR) $(BUILD_DIR)/$(PROGRAM_EXE)
	@echo "=========================="
	@echo "==> Run The Program"
	@echo "=========================="
#	Run the program
	@./$(BUILD_DIR)/$(PROGRAM_EXE)