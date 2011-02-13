all: plasma

plasma: plasma.c
	gcc plasma.c ..\..\src\vesa.o ..\..\src\pcx.o -I..\..\src -o plasma.exe -O2

clean:
	del plasma.exe
