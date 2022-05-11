all: app

app:
	arm-none-linux-gnueabi-gcc -static -o app app.c

install:
	cp -a app /nfsroot

clean:
	rm -rf *.o
	rm -rf app
