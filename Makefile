
CFLAGS=-Wall -O2
LDFLAGS=-lgps

gpsdate: gpsdate.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	@rm -f gpsdate *.o
