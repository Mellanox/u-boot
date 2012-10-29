
all: ppboot.bin ppboot.dis

ppboot.bin: ppboot 
	$(CROSS_COMPILE)objcopy --gap-fill=0xff -O binary ppboot ppboot.bin
    

ppboot: pInit.o spiFlash2Ram.o    
	$(CROSS_COMPILE)ld -Bstatic -T ./ppboot.lds -EB -Ttext 0xFFFFE000 pInit.o spiFlash2Ram.o \
		-Map ppboot.map -o ppboot

ppboot.dis: ppboot
	$(CROSS_COMPILE)objdump -x -d ppboot > ppboot.dis

pInit.o: pInit.S
	$(CROSS_COMPILE)gcc  -D__ASSEMBLY__  -I../../../include \
		-Os -mA7 -c -o pInit.o pInit.S

spiFlash2Ram.o: spiFlash2Ram.c
	$(CROSS_COMPILE)gcc  -D__ASSEMBLY__  -Os -mA7 -I../../../include -c -o spiFlash2Ram.o spiFlash2Ram.c


clean: 
	rm -f *.o
	rm -f ppboot ppboot.bin ppboot.dis ppboot.map
