
CXX = c++

STD = -std=c++23

CXX_FLAGS = -Wall -Wextra -Werror -g $(STD)

NAME = vilgax

INCLUDE = -Isrc/ \
	-Isrc/baselib/ \
	-Isrc/config/ \
	-Isrc/core \

SRC = src/main.cpp \
	src/core/master.cpp \
	src/core/server.cpp \
	src/core/worker.cpp \
	src/config/config.cpp \
	src/config/lexer.cpp \
	src/config/parser.cpp \
	src/baselib/buffer.cpp \
	src/baselib/file.cpp \
	src/baselib/random.cpp \
	src/baselib/string.cpp \
	src/baselib/time.cpp \

OBJECT_DIR = obj

OBJECTS = $(addprefix $(OBJECT_DIR)/, $(SRC:.cpp=.o))

all: $(NAME)

fast: fclean
	make -j4

$(OBJECT_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXX_FLAGS) $(INCLUDE) -c $< -o $@
	@printf "\t%-30s -> %s\n" $(notdir $<) $(notdir $@)

$(NAME): $(OBJECTS)
	@$(CXX) $(CXX_FLAGS) $(INCLUDE) $(OBJECTS) -o $(NAME)
	@echo "Build complete: $(NAME)"

clean:
	rm -fr $(OBJECT_DIR)

fclean: clean
	rm -fr $(NAME)

re: fclean all

.PHONY: all clean fclean re

