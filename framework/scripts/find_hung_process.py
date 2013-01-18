#!/usr/bin/python

# This script can be used to figure out if a job on a cluster is hung.  If all goes well, it'll print the unique
# stack traces out.

import sys
import re
import subprocess

def main():
  if len(sys.argv) != 3:
    print "Usage: " + sys.argv[0] + " <PBS Job num> <Application>"
    sys.exit(1)

  # The PBS job number and the application should be passed on the command line
  job_num = sys.argv[1]
  application = sys.argv[2]

  command = "qstat -n " + job_num

  p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
  output = p.communicate()[0]

  hosts = []

  # The machine name should go here!
  f = re.findall("(fission-\d{4})", output)
  for i in f:
    hosts.append(i)

  matcreates = 0
  bad_hosts = {}

  unique_stack_traces = []
  regex = re.compile("^#0", re.M | re.S)
  for host in hosts:
    command = "ssh " + host + " \"ps -e | grep " + application + " | awk '{print \$1}' | xargs -I {} gdb --batch --pid={} -ex bt 2>&1 | grep '^#' \""
    p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
    output = p.communicate()[0]

    # Python FAIL - We have to re-glue the tokens we threw away from our split (Perl 1 : Python 0)
    traces = ["#0" + trace for trace in regex.split(output)]
    for trace in traces:
      unique = True
      for bt in unique_stack_traces:
        if compare_traces(trace, bt):
          unique = False

      if unique:
        unique_stack_traces.append(trace)

  print "Unique Stack Traces"
  for bt in unique_stack_traces[1:]:
    print "**********************************\n" + bt

def compare_traces(trace1, trace2):
  lines1 = trace1.split("\n")
  lines2 = trace2.split("\n")

  if len(lines1) != len(lines2):
    return False

  # Only compare the stack trace part - not the memoery addresses
  for i in xrange(len(lines1)):
    line1 = lines1[i].split()[2:]
    line2 = lines2[i].split()[2:]

    if line1 != line2:
      return False

  return True

if __name__ == '__main__':
  main()
