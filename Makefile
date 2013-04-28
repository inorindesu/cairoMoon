LUA_PATH=$(HOME)/applications/lua-5.2.1

buildBin: ./build/cairoMoon

run: ./build/cairoMoon
	./build/cairoMoon

./build/cairoMoon: main.c
	mkdir -p ./build/
	gcc -std=c99 `pkg-config --libs --cflags cairo cairo-pdf lua` main.c -o ./build/cairoMoon

.PHONY: clean
clean:
	rm -r ./build/

