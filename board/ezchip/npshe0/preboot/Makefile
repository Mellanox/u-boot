
all: pre-boot.bin preboot.dis

pre-boot.bin: preboot 
	$(CROSS_COMPILE)objcopy --gap-fill=0xff -O binary preboot pre-boot.bin
    

preboot: preStart.o loadRunUboot.o    
	$(CROSS_COMPILE)ld -Bstatic -T ./preboot.lds -EB -Ttext 0xFFFFE800 preStart.o loadRunUboot.o \
		-Map preboot.map -o preboot

preboot.dis: preboot
	$(CROSS_COMPILE)objdump -x -d preboot > preboot.dis

preStart.o: preStart.S
	$(CROSS_COMPILE)gcc  -D__ASSEMBLY__ -I../../../include \
		-Os -mA7 -c -o preStart.o preStart.S

loadRunUboot.o: loadRunUboot.c
	$(CROSS_COMPILE)gcc  -D__ASSEMBLY__ -O3 -mA7 -I../../../include -c -o loadRunUboot.o loadRunUboot.c


clean: 
	rm -f *.o
	rm -f preboot pre-boot.bin preboot.dis preboot.map
