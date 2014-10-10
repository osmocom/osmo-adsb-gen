

all:	adsb_gen

adsb_gen: adsb_gen.c
	$(CC) -Wall $^ -o $@ -losmocore

clean:
	rm -f adsb_gen adsb_gen.o
