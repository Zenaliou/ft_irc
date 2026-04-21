CC      = c++
CFLAGS  = -Wall -Werror -Wextra -std=c++98 -Isrcs -g3

SRC_DIR     = srcs/
OBJ_DIR     = obj/
INCLUDE_DIR = includes/

SRCS = $(shell find $(SRC_DIR) -name '*.cpp')
OBJS = $(SRCS:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)
DEPS = $(OBJS:.o=.d)

NAME = ircserver

GREEN   = \033[1;32m
RED     = \033[1;31m
YELLOW  = \033[1;33m
BLUE    = \033[1;34m
CYAN    = \033[1;36m
NC      = \033[0m

all: $(NAME)
	@echo "$(CYAN)🚀 Tout est prêt !$(NC)"

$(NAME): $(OBJ_DIR) $(OBJS)
	@echo "$(YELLOW)⏳ Compilation de $(NAME)...$(NC)"
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)✓ Compilation de $(NAME) réussie!$(NC)"

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@echo "$(BLUE)📁 Dossier obj créé.$(NC)"

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	@rm -rf $(OBJ_DIR)
	@echo "$(RED)🧹 Fichiers objets supprimés.$(NC)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(RED)🗑️ Exécutable $(NAME) supprimé.$(NC)"

re: fclean all
	@echo "$(CYAN)🔁 Recompilation terminée.$(NC)"

.PHONY: all clean fclean re
