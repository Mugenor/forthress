ASM			= nasm
FLAGS		= -felf64 -Isrc/ 
LINKER 		= ld

all: bin/forthress
	
bin/forthress: obj/forthress.o obj/lib.o
	mkdir -p bin 
	$(LINKER) -o bin/forthress obj/forthress.o obj/lib.o

obj/forthress.o: src/forthress.asm src/macro.inc src/words.inc src/lib.inc src/lib.asm
	mkdir -p obj
	$(ASM) $(FLAGS) src/forthress.asm -o obj/forthress.o

obj/util.o: src/lib.inc src/lib.asm
	mkdir -p obj
	$(ASM) $(FLAGS) src/lib.asm -o obj/lib.o
clean: 
	rm -rf build obj

.PHONY: clean

