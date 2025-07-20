
all:
	gcc -O2 -c brickgame.c -o build/brickgame.o
	gcc -O2 -c loader.c -o build/loader.o
	gcc -O2 -c launcher.c -o build/launcher.o
	gcc -O2 -c games/snake.c -o build/snake.o
	gcc -O2 -c games/tetris.c -o build/tetris.o
	gcc -O2 -c games/example_game.c -o build/example_game.o
	gcc  build/brickgame.o build/loader.o build/launcher.o build/snake.o build/tetris.o build/example_game.o -lX11 -o build/main
