// Copyright(C) 2008 Sandia Corporation.  Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
// certain rights in this software
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of Sandia Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/times.h>
#include <time.h>

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <cstdio>

using namespace std;

void add_to_log(const char *my_name)
{
#define LEN 512
  char time_string[LEN];
  char log_string[LEN];

  double u_time, s_time;
  struct utsname sys_info;
  
  /* Don't log information if this environment variable is set */
  if (getenv("SEACAS_NO_LOGGING") != NULL) {
    fprintf(stderr, "SEACAS Audit logging disabled via SEACAS_NO_LOGGING setting.\n");
    return;
  }
  
  /* Now try to find the $ACCESS/etc/audit.log file */
  /* Don't need to try too hard since information is not critical; just useful */
  char *access_dir = getenv("ACCESS");
  if (access_dir != NULL) {
    char filename[LEN];
    sprintf(filename, "%s/etc/audit.log", access_dir);
    if (0 == access(filename, W_OK)) {
      FILE *audit = fopen(filename, "a");
      if (audit != NULL) {

	char *username = getlogin();
	if (username == NULL) {
	  username = getenv("LOGNAME");
	}

	const char *codename = strrchr (my_name, '/');
	if (codename == NULL)
	  codename = my_name;
	else
	  codename++;

	{
	  time_t calendar_time = time(NULL);
	  struct tm *local_time = localtime(&calendar_time);
	  strftime(time_string, LEN, "%a %b %d %H:%M:%S %Z %Y", local_time);
	}

	{
	  int ticks_per_second;
	  struct tms time_buf;
	  times(&time_buf);
	  ticks_per_second = sysconf(_SC_CLK_TCK);
	  u_time = (double)(time_buf.tms_utime + time_buf.tms_cutime) / ticks_per_second;
	  s_time = (double)(time_buf.tms_stime + time_buf.tms_cstime) / ticks_per_second;
	}
  
	uname(&sys_info);

	sprintf(log_string, "%s %s %s %.3fu %.3fs 0:00.00 0.0%% 0+0k 0+0io 0pf+0w %s\n",
		my_name, username, time_string, u_time, s_time, sys_info.nodename);

	fprintf(audit, "%s", log_string);
	fclose(audit);
      }
    }
  }
}
