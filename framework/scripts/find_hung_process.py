#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script can be used to figure out if a job on a cluster is hung.  If all goes well, it'll print the unique
# stack traces out.

import sys, os, re, subprocess
from tempfile import TemporaryFile
from optparse import OptionParser, OptionGroup, Values

##################################################################
# Modify the following variable(s) for your cluster or use one of the versions below
### FISSION
#node_name_pattern = re.compile("(fission-\d{4})")
#pstack_binary = 'pstack'
### BECHLER
#node_name_pattern = re.compile("(b\d{2})")
#pstack_binary = 'pstack'
### FALCON
node_name_pattern = re.compile("(r\di\dn\d{1,2})")
pstack_binary = 'gstack'
##################################################################

def generateTraces(job_num, application_name, num_hosts):
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
    if num_hosts == 0:
        num_hosts = len(hosts)

    for i in range(len(hosts)):
        if i >= num_hosts:
            continue

        #command = "ssh " + host + " \"ps -e | grep " + application_name + " | awk '{print \$1}' | xargs -I {} gdb --batch --pid={} -ex bt 2>&1 | grep '^#' \""
        command = "ssh " + hosts[i] + " \"ps -e | grep " + application_name + " | awk '{print \$1}' | xargs -I '{}' sh -c 'echo Host: " + hosts[i] + " PID: {}; " + pstack_binary + " {}; printf '*%.0s' {1..80}; echo' \""
        f = TemporaryFile()
        p = subprocess.Popen(command, stdout=f, close_fds=False, shell=True)
        jobs.append((p, f))

    # Now process the output from each of the jobs
    traces = []
    for (p, f) in jobs:
        p.wait()
        f.seek(0)
        output = f.read()
        f.close()

        # strip blank lines
        output = os.linesep.join([s for s in output.splitlines() if s])

        traces.extend(splitTraces(output))

    return traces

def readTracesFromFile(filename):
    f = open(filename)
    data = f.read()
    return splitTraces(data)

def splitTraces(trace_string):
    trace_regex = re.compile("^\**\n", re.M)
    traces = trace_regex.split(trace_string)

#  # Only keep lines beginning with a #
#  throw_away = re.compile("^[^#].*", re.M)
#  traces = [throw_away.sub("", trace) for trace in traces]

    return traces

# Process the individual traces
def processTraces(traces, num_lines_to_keep):
    unique_stack_traces = {}
    last_lines_regex = re.compile("(?:.*\n){" + str(num_lines_to_keep) + "}\Z", re.M)
    host_regex = re.compile("^(Host.*)", re.M)

    for trace in traces:
        if len(trace) == 0:
            continue

        # Grab the host and PID
        m = host_regex.search(trace)
        if m:
            host_pid = m.group(1)

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
            unique_stack_traces[trace] = [host_pid]
        else:
            unique_stack_traces[unique].append(host_pid)

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
    parser = OptionParser(usage='Usage: %prog [options] <PBS Job num> <Application>')
    parser.add_option('-s', '--stacks', action='store', dest='stacks', type='int', default=0, help="The number of stack frames to keep and compare for uniqueness (Default: ALL)")
    parser.add_option('-n', '--hosts', action='store', dest='hosts', type='int', default=0, help="The number of hosts to visit (Default: ALL)")
    parser.add_option('-f', '--force', action='store_true', dest='force', default=False, help="Whether or not to force a regen if a cache file exists")
    (options, args) = parser.parse_args()

    if len(args) != 2:
        parser.print_help()
        sys.exit(1)

    # The PBS job number and the application should be passed on the command line
    # Additionally, an optional argument of the number of frames to keep (compare) may be passed
    job_num = args[0]
    application = args[1]
    num_to_keep = options.stacks
    num_hosts = options.hosts

    # first see if there is a cache file available
    cache_filename = application + '.' + job_num + '.cache'

    traces = []
    if not os.path.exists(cache_filename) or options.force:
        traces = generateTraces(job_num, application, options.hosts)

        # Cache the restuls to a file
        cache_file = open(cache_filename, 'w')
        for trace in traces:
            cache_file.write(trace + "*"*80 + "\n")
        cache_file.write("\n")
        cache_file.close()

    # Process the traces to collapse them into unique stacks
    traces = readTracesFromFile(cache_filename)
    unique_stack_traces = processTraces(traces, num_to_keep)

    print "Unique Stack Traces"
    for trace, count in unique_stack_traces.iteritems():
        print "*"*80 + "\nCount: " + str(len(count)) + "\n"
        if len(count) < 10:
            print "\n".join(count)
        print "\n" + trace


if __name__ == '__main__':
    main()
