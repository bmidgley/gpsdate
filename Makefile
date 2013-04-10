
CFLAGS=-Wall -O2

gpsdate: gpsdate.o
	$(CC) $(LDFLAGS) -o $@ $^ -lgps

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	@rm -f gpsdate *.o
