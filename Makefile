CC = arc-elf32-gcc
CFLAGS = -ffreestanding -nostdlib -lgcc -mno-sdata
LDFLAGS = -M,-T
ODIR=obj
 _OBJ = main.o ui.o string.o
OBJ  = $(patsubst %,$(ODIR)/%,$(_OBJ))
default: sd

$(OBJ): $(ODIR)/%.o: %.c
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
elfCart: $(OBJ) $(SOBJ)
	$(CC) -o $@ $^ $(CFLAGS) -Wl,$(LDFLAGS),leapsterCart.ld
elfSD: $(OBJ) $(SOBJ)
	$(CC) -o $@ $^ $(CFLAGS) -Wl,$(LDFLAGS),leapsterSD.ld
sd: elfSD
	./tools/binToRib ./tools/SDMenu.bin ./elfSD sd
clean:
	rm -rf $(ODIR)/*
	rm -f ./elfSD
