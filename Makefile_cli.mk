MAKE_NAME = Makefile_cli.make

NAME = na_cli

CC     = gcc
LD     = $(CC)
DEPEND = makedepend

SRC_DIR = ./src
OBJ_DIR = ./obj

SRC_FULL_PATH =\
	src/cli/cli.c\
	src/ipc/ipc.c\
	src/main_cli.c\


SRC = $(notdir $(SRC_FULL_PATH))

OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))

INC_DIRS =\
	src/cli\
	src/ipc\

INC = $(foreach inc_dir, $(INC_DIRS), $(addsuffix /*.h, $(wildcard $(inc_dir))))

CFLAGS = -g
IFLAGS = $(foreach inc_dir, $(INC_DIRS), $(addprefix -I, $(wildcard $(inc_dir))))
LFLAGS = -lpcap -pthread

all: $(NAME)

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
# 	$(RM) -r $(OBJ_DIR)

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

vpath %.c $(SRC_DIR)
vpath %.c $(SRC_DIR)/cli
vpath %.c $(SRC_DIR)/ipc

.PHONY: all clean fclean re depend test