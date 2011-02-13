all: pcx.o vesa.o

pcx.o: pcx.c
	gcc pcx.c -c -O2

vesa.o: vesa.c
	gcc vesa.c -c -O2

clean:
	del *.o
	del *.swp
