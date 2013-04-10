/* gpsdate - small utility to set system RTC based on gpsd time 
 * (C) 2013 by sysmocom - s.f.m.c. GmbH, Author: Harald Welte
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* The idea of this program is that you run it once at system boot time,
 * to set the local RTC to the time received by GPS.  Further synchronization
 * during system runtime is then handled by ntpd, interfacing with gpsd using
 * the ntp shared memory protocol.
 *
 * However, ntpd is unable to accept a GPS time that's off by more than four
 * hours from the system RTC, so initial synchronization has to be done
 * externally.  'ntpdate' is the usual option, but doesn't work if you're
 * offline.  Thus, this gpsdate utilith was created to fill the gap.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

#include <gps.h>

static struct gps_data_t gpsdata;

static void callback(struct gps_data_t *gpsdata)
{
	struct timeval tv;
	int rc;

	if (!(gpsdata->set & TIME_SET))
		return;

	tv.tv_sec = gpsdata->fix.time;
	/* FIXME: use the fractional part for microseconds */
	tv.tv_usec = 0;

	rc = settimeofday(&tv, NULL);
	gps_close(gpsdata);
	if (rc == 0)
		exit(EXIT_SUCCESS);
	else
		exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	char *host = "localhost";

	if (argc > 1)
		host = argv[1];

	if (gps_open(host, DEFAULT_GPSD_PORT, &gpsdata)) {
		fprintf(stderr, "no gpsd running or network error: %d, %s\n",
			errno, gps_errstr(errno));
		exit(EXIT_FAILURE);
	}

	gps_stream(&gpsdata, WATCH_ENABLE|WATCH_JSON, NULL);

	gps_mainloop(&gpsdata, 5000000, callback);

	gps_close(&gpsdata);

	exit(EXIT_SUCCESS);
}
