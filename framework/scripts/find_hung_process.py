#!/usr/bin/env python

# This script can be used to figure out if a job on a cluster is hung.  If all goes well, it'll print the unique
# stack traces out.

import sys, os, re, subprocess
from tempfile import TemporaryFile

##################################################################
# Modify the following variable for your cluster
node_name_pattern = re.compile("(fission-\d{4})")
##################################################################

def generateTraces(job_num, application_name):
  command = "qstat -n " + job_num

  p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
  output = p.communicate()[0]

  # The lists of hosts
  hosts = []
  # The array of jobs
  jobs = []

  # The machine name should go here!
  host_strs = node_name_pattern.findall(output)
  for i in host_strs:
    hosts.append(i)

  # Launch all the jobs
  for host in hosts:
    command = "ssh " + host + " \"ps -e | grep " + application_name + " | awk '{print \$1}' | xargs -I {} gdb --batch --pid={} -ex bt 2>&1 | grep '^#' \""
    f = TemporaryFile()
    p = subprocess.Popen(command, stdout=f, close_fds=False, shell=True)
    jobs.append((p, f))

  # Now process the output from each of the jobs
  traces = []
  trace_regex = re.compile("^#0", re.M | re.S)
  for (p, f) in jobs:
    p.wait()
    f.seek(0)
    output = f.read()
    f.close()

    # Python FAIL - We have to re-glue the tokens we threw away from our split (Perl 1 : Python 0)
    traces.extend(["#0" + trace for trace in trace_regex.split(output) if len(trace)])

  return traces

def readTracesFromFile(filename):
  trace_regex = re.compile("^\**", re.M)
  f = open(filename)
  data = f.read()
  traces = trace_regex.split(data)

  # Only keep lines beginning with a #
  throw_away = re.compile("^[^#].*", re.M)
  traces = [throw_away.sub("", trace) for trace in traces]

  return traces

# Process the individual traces
def processTraces(traces, num_lines_to_keep):
  unique_stack_traces = {}
  last_lines_regex = re.compile("(?:.*\n){" + str(num_lines_to_keep) + "}\Z", re.M)

  for trace in traces:
    if len(trace) == 0:
      continue

    # If the user requested to save only the last few lines, do that here
    if num_lines_to_keep:
      m = last_lines_regex.search(trace)
      if m:
        trace = m.group(0)

    unique = ''
    for bt in unique_stack_traces:
      if compareTraces(trace, bt):
        unique = bt

    if unique == '':
      unique_stack_traces[trace] = 1
    else:
      unique_stack_traces[unique] += 1

  return unique_stack_traces

def compareTraces(trace1, trace2):
  lines1 = trace1.split("\n")
  lines2 = trace2.split("\n")

  if len(lines1) != len(lines2):
    return False

  # Only compare the stack trace part - not the memory addresses
  # Note this subroutine may need tweaking if the stack trace is different
  # on the current machine
  memory_re = re.compile("0x[0-9a-f]*")
  for i in xrange(len(lines1)):
    line1 = lines1[i].split()[2:]
    line2 = lines2[i].split()[2:]

    # Let's strip out all the memory addresses too
    line1 = [memory_re.sub("0x...", line) for line in line1]
    line2 = [memory_re.sub("0x...", line) for line in line2]

    if line1 != line2:
      return False

  return True

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

  # first see if there is a cache file available
  cache_filename = application + '.' + job_num + '.cache'

  traces = []
  if os.path.exists(cache_filename):
    traces = readTracesFromFile(cache_filename)
  else:
    traces = generateTraces(job_num, application)

    # Cache the restuls to a file
    cache_file = open(cache_filename, 'w')
    for trace in traces:
      cache_file.write("**********************************\n" + trace)
    cache_file.close()

  # Process the traces to collapse them into unique stacks
  unique_stack_traces = processTraces(traces, num_to_keep)

  print "Unique Stack Traces"
  for trace, count in unique_stack_traces.iteritems():
    print "**********************************\nCount: " + str(count) + "\n" + trace


if __name__ == '__main__':
  main()
