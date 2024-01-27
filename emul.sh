rm ./test/*.o
rm ./test/*.dat
rm ./test/*.hex
rm ./test/*.txt
./asembler -o main.o main.s
./asembler -o math.o math.s
./asembler -o handler.o handler.s
./asembler -o isr_timer.o isr_timer.s
./asembler -o isr_terminal.o isr_terminal.s
./asembler -o isr_software.o isr_software.s
./linker -hex -place=my_code@0x40000000 -place=math@0xF0000000 -o linked.hex handler.o math.o main.o isr_terminal.o isr_timer.o isr_software.o
./emulator linked.hex