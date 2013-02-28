#!/usr/bin/env python

# This script can be used to figure out if a job on a cluster is hung.  If all goes well, it'll print the unique
# stack traces out.

import sys
import re
import subprocess
from tempfile import TemporaryFile

def main():
  if len(sys.argv) < 3 or len(sys.argv) > 4:
    print "Usage: " + sys.argv[0] + " <PBS Job num> <Application> [num frames to keep]"
    sys.exit(1)

  # The PBS job number and the application should be passed on the command line
  # Additionally, an optional argument of the number of frames to keep (compare) may be passed
  job_num = sys.argv[1]
  application = sys.argv[2]
  num_to_keep = 0
  if len(sys.argv) == 4:
    num_to_keep = int(sys.argv[3])

  command = "qstat -n " + job_num

  p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
  output = p.communicate()[0]

  # The lists of hosts
  hosts = []
  # The array of jobs
  jobs = []

  # The machine name should go here!
  host_strs = re.findall("(fission-\d{4})", output)
  for i in host_strs:
    hosts.append(i)

  unique_stack_traces = {}
  trace_regex = re.compile("^#0", re.M | re.S)
  last_lines_regex = re.compile("(?:.*\n){" + str(num_to_keep) + "}\Z", re.M)

  # Launch all the jobs
  for host in hosts:
    command = "ssh " + host + " \"ps -e | grep " + application + " | awk '{print \$1}' | xargs -I {} gdb --batch --pid={} -ex bt 2>&1 | grep '^#' \""
    f = TemporaryFile()
    p = subprocess.Popen(command, stdout=f, close_fds=False, shell=True)
    jobs.append((p, f))

  # Now process the output from each of the jobs
  for (p, f) in jobs:
    p.wait()
    f.seek(0)
    output = f.read()
    f.close()

    # Python FAIL - We have to re-glue the tokens we threw away from our split (Perl 1 : Python 0)
    traces = ["#0" + trace for trace in trace_regex.split(output)]
    for trace in traces:
      if trace == '#0': # More Python FAIL - why you suck so bad?
        continue

      # If the user requested to save only the last few lines, do that here
      if num_to_keep:
        m = last_lines_regex.search(trace)
        if m:
          trace = m.group(0)

      unique = ''
      for bt in unique_stack_traces:
        if compare_traces(trace, bt):
          unique = bt

      if unique == '':
        unique_stack_traces[trace] = 1
      else:
        unique_stack_traces[unique] += 1


  print "Unique Stack Traces"
  for trace, count in unique_stack_traces.iteritems():
    print "**********************************\nCount: " + str(count) + "\n" + trace

def compare_traces(trace1, trace2):
  lines1 = trace1.split("\n")
  lines2 = trace2.split("\n")

  if len(lines1) != len(lines2):
    return False

  # Only compare the stack trace part - not the memory addresses
  # Note this subroutine may need tweaking if the stack trace is different
  # on the current machine
  for i in xrange(len(lines1)):
    line1 = lines1[i].split()[2:]
    line2 = lines2[i].split()[2:]

    if line1 != line2:
      return False

  return True

if __name__ == '__main__':
  main()
