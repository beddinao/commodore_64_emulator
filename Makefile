all: 
	@cmake -B build
	@cmake --build build
	@mv build/c64_emu .

clean:
	rm -fr build c64_emu
