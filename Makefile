all:
	gcc \
	-Wall \
	-o ./fabrik_demo \
	./src/*.c \
	-I./include -L./lib \
	-lraylib -lpthread -lm -ldl
