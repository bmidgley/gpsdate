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
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/time.h>

#include <gps.h>

#define NUM_RETRIES	60	/* Number of gpsd re-connects */
#define RETRY_SLEEP	1	/* Seconds to sleep between re-connects */

static struct gps_data_t gpsdata;

static void callback(struct gps_data_t *gpsdata)
{
	struct timeval tv;
	time_t time;
	int rc;

	if (!(gpsdata->set & TIME_SET))
		return;

	tv.tv_sec = gpsdata->fix.time;
	/* FIXME: use the fractional part for microseconds */
	tv.tv_usec = 0;

	time = tv.tv_sec;

	rc = settimeofday(&tv, NULL);
	gps_close(gpsdata);
	if (rc == 0) {
		syslog(LOG_NOTICE, "Successfully set RTC time to GPSD time:"
			" %s", ctime(&time));
		closelog();
		exit(EXIT_SUCCESS);
	} else {
		syslog(LOG_ERR, "Error setting RTC: %d (%s)\n",
			errno, strerror(errno));
		closelog();
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	char *host = "localhost";
	int i, rc;

	openlog("gpsdate", LOG_PERROR, LOG_CRON);

	if (argc > 1)
		host = argv[1];

	for (i = 1; i <= NUM_RETRIES; i++) {
		printf("Attempt #%d to connect to gpsd at %s...\n", i, host);
		rc = gps_open(host, DEFAULT_GPSD_PORT, &gpsdata);
		if (!rc)
			break;
		sleep(RETRY_SLEEP);
	}

	if (rc) {
		syslog(LOG_ERR, "no gpsd running or network error: %d, %s\n",
			errno, gps_errstr(errno));
		closelog();
		exit(EXIT_FAILURE);
	}

	gps_stream(&gpsdata, WATCH_ENABLE|WATCH_JSON, NULL);

	gps_mainloop(&gpsdata, 5000000, callback);

	gps_close(&gpsdata);

	closelog();
	exit(EXIT_SUCCESS);
}
