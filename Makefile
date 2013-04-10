
CFLAGS=-Wall -O2

gpsdate: gpsdate.o
	$(CC) $(LDFLAGS) -lgps -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	@rm -f gpsdate *.o
