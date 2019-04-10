MAKE_NAME = Makefile

NAME = na

CC     = gcc
LD     = $(CC)
DEPEND = makedepend

SRC_DIR = ./src
OBJ_DIR = ./obj

SRC_FULL_PATH =\
	src/nstat/nstat.c\
	src/daemon/daemon.c\
	src/sniffer/sniffer.c\
	src/avltree/avl.c\
	src/ipc/ipc.c\
	src/cli_handler.c\
	src/main.c\


SRC = $(notdir $(SRC_FULL_PATH))

OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))

INC_DIRS =\
	src/avltree\
	src/daemon\
	src/nstat\
	src/sniffer\
	src/cli_handler\
	src/ipc\

INC = $(foreach inc_dir, $(INC_DIRS), $(addsuffix /*.h, $(wildcard $(inc_dir))))

CFLAGS = -g -Wshadow
IFLAGS = $(foreach inc_dir, $(INC_DIRS), $(addprefix -I, $(wildcard $(inc_dir))))
LFLAGS = -lpcap -pthread

all:
	make $(NAME)

$(NAME): $(OBJ)
	$(LD) $(OBJ) $(LFLAGS) -o $(NAME)

$(OBJ_DIR)/%.o: %.c $(INC)
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

$(OBJ): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)
	$(RM) -r $(OBJ_DIR)

re: fclean all

depend:
	@touch __tmp_makedepend_out__
	@$(DEPEND) -f __tmp_makedepend_out__ -- $(IFLAGS) -- $(SRC_FULL_PATH)
	@sed -i '' 's!\(.*\/\)*\(.*:\)!$(OBJ_DIR)/\2!g' __tmp_makedepend_out__
	@sed '/^# DO NOT DELETE/q' $(MAKE_NAME) > __tmp_makefile__
	@cat __tmp_makefile__ > $(MAKE_NAME)
	@rm __tmp_makefile__
	@cat __tmp_makedepend_out__ >> $(MAKE_NAME)
	@rm __tmp_makedepend_out__
	@rm __tmp_makedepend_out__.bak
	@echo "*** make depend ***"

cli:
	make -f Makefile_cli.mk

vpath %.c $(SRC_DIR)
vpath %.c $(SRC_DIR)/avltree
vpath %.c $(SRC_DIR)/daemon
vpath %.c $(SRC_DIR)/nstat
vpath %.c $(SRC_DIR)/sniffer
vpath %.c $(SRC_DIR)/ipc
vpath %.c $(SRC_DIR)/cli_handler

.PHONY: all clean fclean re depend test