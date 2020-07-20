.PHONY: all clean install uninstall backup

CC = gcc -Wall -O0

INSTALL_DIR = ~/scripts/bin
CONFIG_DIR = ~/.config
THEMER_DIR = $(CONFIG_DIR)/themer
BACKUP_DIR = $(CONFIG_DIR)/themer.bak
BIN_DIR = bin

all: $(BIN_DIR)/themer

clean:
	rm -r $(BIN_DIR)

install: $(BIN_DIR)/themer | $(THEMER_DIR)
	cp $(BIN_DIR)/themer $(INSTALL_DIR)

uninstall:
	make backup
	@echo Uninstalling...
	rm -rf $(BIN_DIR)
	rm -f $(INSTALL_DIR)/themer
	rm -rf $(THEMER_DIR)

backup:
	@echo Backing up $(THEMER_DIR) to $(BACKUP_DIR)

ifneq ("$(wildcard $(THEMER_DIR))", "")
	cp $(THEMER_DIR) $(BACKUP_DIR) -rTv --backup=numbered
else
	@echo No $(THEMER_DIR) to backup!
endif

$(BIN_DIR)/themer: bin/themer.o bin/logger.o bin/parser.o | bin
	$(CC) -o $(BIN_DIR)/themer bin/themer.o bin/logger.o bin/parser.o

$(BIN_DIR)/themer.o: source/themer.c | bin
	$(CC) -o $(BIN_DIR)/themer.o -I header/ source/themer.c -c

$(BIN_DIR)/logger.o: header/logger.h source/logger.c | bin
	$(CC) -o $(BIN_DIR)/logger.o -I header/ source/logger.c -c

$(BIN_DIR)/parser.o: header/parser.h source/parser.c | bin
	$(CC) -o $(BIN_DIR)/parser.o -I header/ source/parser.c -c

$(BIN_DIR):
	mkdir $(BIN_DIR)

$(THEMER_DIR):
	mkdir $(THEMER_DIR)
	mkdir $(THEMER_DIR)/src
	mkdir $(THEMER_DIR)/$(BIN_DIR)
	touch $(THEMER_DIR)/themer-post.sh
