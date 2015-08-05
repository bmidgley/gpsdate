
CFLAGS=-Wall -O2

all: gpsdate gps-watchdog

gpsdate: gpsdate.o
	$(CC) $(LDFLAGS) -o $@ $^ -lgps

gps-watchdog: gps-watchdog.o
	$(CC) $(LDFLAGS) -o $@ $^ -lgps

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	@rm -f gpsdate gps-watchdog *.o
