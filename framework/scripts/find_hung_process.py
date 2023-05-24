#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
This script can be used to figure out if a job on a cluster is hung.
If all goes well, it'll print the unique stack traces out.

To use this script, you first need to submit a job on your cluster, where
the binary you are probing doesn't have symbols stripped. If you are working
with MOOSE many of the modes will work, the best two are METHOD=dbg or
METHOD=devel. Make sure your job is running.

GENERATE TRACES STAGE
This script will run first run qstat -n <job number> to figure out which
nodes your job is running on. If you need to modify that command for your
cluster, you'll find it in "generate traces". Note: This script is currently
only designed to work with PBS, but could be easily adapted to other queuing
systems.

Next, we use a RegEx to parse through the output of qstat to find the nodes
where your job is running. Check the patterns below and make sure they work
for your cluster (i.e. node_name_pattern).

Once the script has all the node names down, it'll assemble a bunch of
complex ssh commands to login and pull back the traces based on PID. It
figures out the PIDs by grepping `ps -ef` for your application name so
make sure you know how your application appears on one of the running
nodes.

Finally, double check the stack binary "PSTACK_BINARY" for your cluster.

If all of this works, the script will use several threads to get stack
traces from all of your ranks and dump them to a local cache file. It's
just plan ASCII so you can "cat" it to look at the contents. You'll
see the node names and a series of stacks for each rank in that file
with handly line separators.

PROCESS TRACES STAGE
If all of the above works, you'll see the cache file, don't expect
this stage to work if you don't have a valid cache file on your disk
after running this script.

THis stage starts by opening and reading the TRACES. It does splitting
and pulling off the names off each stack (the node names where the stack
trac was pulled). It then attempts to "bucket" the stacks into unique
traces. If all the processors have the exact same stack trace (i.e.
perfectly syncronized), You'll see only one stack trace with the total
count of how many ranks reported _that_ trace. If however you have
divergence, you may have two unique traces or perhaps even more. The
script will attempt to find the first location of divergence, which
if there is a logic bug should help you pin down the location of where
to start searching for a problem.

If for whatever reason, you want to override how many frames to analyze
you can rerun the script passing in the name of the cache file and how many
stack traces to look at during the "bucketing" process
(e.g. find_hung_processes.py -f my_app.1234.cache -s 10).
The command above will open the given cache file and only pay attention to
the lowest 10 frames for bucketing purposes.
"""

import sys
import os
import re
import subprocess
import argparse
import math
from tempfile import TemporaryFile
from time import sleep
from multiprocessing.pool import ThreadPool
import threading # for thread locking and thread timers

##################################################################
# Modify the following variable(s) for your cluster or use one of the versions below
### FISSION
# node_name_pattern = re.compile('(fission-\d{4})')
# pstack_binary = 'pstack'
### BECHLER
# node_name_pattern = re.compile('(b\d{2})')
# pstack_binary = 'pstack'
### FALCON
node_name_pattern = re.compile(r'(r\di\dn\d{1,2})')
PSTACK_BINARY = 'gstack'
##################################################################

class Job:
    """
    Job contains a simple subprocess object.

    Syntax:
      job = Job(['/bin/sleep 20'])
      job.run()
      job.get_process()   # retrieve entire subprocess object
      job.get_stdout()    # retrieve protected stdout
      job.get_stderr()    # retrieve protected stderr
    """
    def __init__(self, command):
        self.__command = command
        self.__process = None
        self.__stdout_file = TemporaryFile()
        self.__stderr_file = TemporaryFile()
    def get_process(self):
        """
        Return the subprocess object
        """
        return self.__process
    def get_stdout(self):
        """
        Return stdout if process has completed
        """
        if self.get_process().poll() is not None:
            self.__stdout_file.seek(0)
            return self.__stdout_file.read().decode('utf-8')
        return None
    def get_stderr(self):
        """
        Return stderr if process has completed
        """
        if self.get_process().poll() is not None:
            self.__stderr_file.seek(0)
            return self.__stderr_file.read().decode('utf-8')
        return None
    def run(self, exec_timeout=None):
        """
        Execute command and wait for return code
        """
        with subprocess.Popen(self.__command,
                              stdout=self.__stdout_file,
                              stderr=self.__stderr_file,
                              shell=True,
                              encoding='utf-8') as self.__process:
            self.__process.wait(timeout=exec_timeout)
            return self.__process.poll()

class Scheduler:
    """
    A simple threading object handler.

    Syntax:
      jobs = [[job], [job]]
      runner = Scheduler()

    .schedule_jobs(jobs) calls object's run method
    .schedule_jobs()     blocks until all jobs launch
    .wait_finish()       blocks until all jobs finish
    .get_finished()      returns
    """
    def __init__(self, max_slots=12, exec_timeout=None):
        self.max_slots = int(max_slots)
        self.worker_pool = ThreadPool(processes=self.max_slots)
        self.thread_lock = threading.Lock()
        self.active = set([])
        self.finished = set([])
        self.__timeout = exec_timeout

    def get_finished(self):
        """
        Return finished jobs
        """
        return self.finished

    def wait_finish(self):
        """
        blocks until all submitted jobs are finished
        """
        while self.active:
            sleep(.5)
        self.worker_pool.close()
        self.worker_pool.join()

    def schedule_jobs(self, jobs):
        """
        Instance one or more Job objects and queue them into the
        thread pool.
        """
        instanced_jobs = []
        for a_job in jobs:
            instanced_jobs.append(Job(a_job))
        if instanced_jobs:
            self.queue_jobs(instanced_jobs)

    def queue_jobs(self, jobs):
        """
        Add all jobs to the thread pool
        """
        for o_job in jobs:
            self.worker_pool.apply_async(self.launch_job, (o_job,))

    def launch_job(self, o_job):
        """
        Run subprocess using provided command list
        """
        try:
            o_job.run(exec_timeout=self.__timeout)
            with self.thread_lock:
                self.finished.add(o_job)
        except KeyboardInterrupt:
            sys.exit(1)



def __schedule_task(jobs, num_threads):
    schedule_jobs = Scheduler(max_slots=num_threads)
    schedule_jobs.schedule_jobs(jobs)
    schedule_jobs.wait_finish()
    return schedule_jobs.get_finished()

def __get_pids(application_name, hosts, num_threads):
    """
    SSH into each host and retrieve PIDs
    return a dictionary of {'hostname' : [pids]}
    """
    jobs = []
    results = {}
    for host in hosts:
        jobs.append(get_sshpids(application_name, host))
    finished_jobs = __schedule_task(jobs, num_threads)
    for job in finished_jobs:
        std_out = job.get_stdout().split()
        results[std_out[0]] = std_out[1:]
    return results

def __get_stacks(hosts_pids, num_threads):
    """
    Iterate over dictionary of hosts, and PIDs and run
    a stack trace on each one, return a list of results.
    """
    jobs = []
    results = []
    for host, pids in hosts_pids.items():
        for pid in pids:
            jobs.append(get_sshstack(host, pid))
    finished_jobs = __schedule_task(jobs, num_threads)
    for job in finished_jobs:
        results.append(job.get_stdout())
    return results

def get_sshpids(application_name, host):
    """
    Generate and return a valid SSH remote ps command
    """
    ps_grep = f'ps -e | grep {application_name}'
    awk_print = r"awk '{print \$1}'"
    return f'ssh {host} "echo {host}; {ps_grep} | {awk_print}"'

def get_sshstack(host, pid):
    """
    Generate and return a valid SSH remote stacktrace command
    """
    return f'ssh {host} "echo Host: {host} PID: {pid}; ' \
           f'{PSTACK_BINARY} {pid}; printf "*%.0s" {{1..80}}; echo"'

def generate_traces(job_num, application_name, num_hosts, num_threads):
    """
    Generate a temporary file with traces and return
    formated results
    """
    hosts = set([])
    a_job = Job(f'qstat -n {job_num}')
    a_job.run()
    results = a_job.get_stdout()
    for i in node_name_pattern.findall(results):
        hosts.add(i)
    if num_hosts:
        # convert back into set
        hosts = set(list(hosts)[:num_hosts])

    # Use a Scheduler to get hosts and PIDs
    hosts_pids = __get_pids(application_name, hosts, num_threads)

    # Use a Scheduler to get stack traces from each
    # host for every PID
    stack_list = __get_stacks(hosts_pids, num_threads)

    traces = []
    for stack in stack_list:
        output = os.linesep.join([s for s in stack.splitlines() if s])
        traces.extend(ProcessTraces.split_traces(output))
    return traces


class BinSearchIndices:
    """
    Utility to return indicies to check for a binary
    search. The next value to try is returned each time
    "next" is called.
    """
    def __init__(self, lower, upper):
        self.lower = lower
        self.upper = upper

    def next_index(self):
        if self.upper < self.lower:
            return -1

        return (self.upper + self.lower) // 2

    def set_direction(self, next_direction):
        if next_direction >= 1:
            self.lower = self.next_index() + 1
        else:
            self.upper = self.next_index() - 1
        return

    def estimated_steps_remaining(self):
        range = abs(self.upper - self.lower)
        if range <= 1:
            return range + 1

        return math.ceil(math.log2(self.upper - self.lower))

class ProcessTraces:
    """
    This class reads a series of stack traces from a file
    and then processes them to determine if processes in an MPI parallel
    job have diverged (having differing stack traces that are due to just
    noise). Memory addresses are stripped.
    """

    def __init__(self):
        self.unique_stack_traces = {}
        self.filename = None
        self.traces = None
        self.host_and_frames_re = re.compile('^(Host.*?)\n(.*)', re.M | re.S)
        self.memory_re = re.compile(r'0x[0-9a-f]*')    # Pattern to match (and remove) memory addresses
        self.frame_num_re = re.compile(r'^#\d.', re.M) # frame numbers IMPORTANT: Always match 2 characters due to fixed width formatting!

    def read_tracesfromfile(self, filename):
        """
        Read tracefile, and return a list of traces
        """
        self.filename = filename
        with open(filename, encoding='utf-8') as t_file:
            data = t_file.read()
            if data == '':
                raise Exception("Cache file is empty - check your job number and application name")
            self.traces = self.split_traces(data)
        return

    @staticmethod
    def split_traces(trace_string):
        """
        Return list of traces based on regular expression:

        Only keep lines beginning with a #
        throw_away = re.compile("^[^#].*", re.M)
        traces = [throw_away.sub("", trace) for trace in traces]
        """
        trace_regex = re.compile(r'^\**\n', re.M)
        return trace_regex.split(trace_string)

    def __longest_trace(self):
        """
        Returns the number of lines in the longest trace
        """
        return max(trace.count('\n') for trace in self.traces)

    def process_traces(self, num_lines_to_keep):
        """
        Process the individual traces into buckets to determine whether processes have diverged from one and other.
        """
        unique_stack_traces = {}
        bin_search_index = None
        last_index = -1
        if num_lines_to_keep == 0:
            bin_search_index = BinSearchIndices(1, self.__longest_trace())
        else:
            # Set the start and end indices to the same to force a single pass
            bin_search_index = BinSearchIndices(num_lines_to_keep, num_lines_to_keep)
            last_index = num_lines_to_keep # Setting stopping criteria

        last_diveged_index = -1
        last_result_diverged = True
        last_diverged_result = {}
        while True:
            num_lines_to_keep = bin_search_index.next_index()

            last_lines_regex = re.compile(fr'(?:.*\n){{{num_lines_to_keep}}}\Z', re.M)
            print(f"Comparing stacks through frame {num_lines_to_keep}. Estimated steps remaining: {bin_search_index.estimated_steps_remaining()}", end=" ")

            self.unique_stack_traces.clear()
            for trace in self.traces:
                if len(trace) == 0:
                    continue

                # Grab the host and PID and strip
                tmp_trace = self.host_and_frames_re.search(trace)
                if tmp_trace:
                    host_pid = tmp_trace.group(1)
                    trace = tmp_trace.group(2)

                # If the user requested to save only the last few lines, do that here
                if num_lines_to_keep:
                    tmp_trace = last_lines_regex.search(trace)
                    if tmp_trace:
                        trace = tmp_trace.group(0)

                unique = ''
                for back_trace in self.unique_stack_traces:
                    if self.compare_traces(trace, back_trace):
                        unique = back_trace

                if unique == '':
                    self.unique_stack_traces[trace] = [host_pid]
                else:
                    self.unique_stack_traces[unique].append(host_pid)

            # Figure out if we need to search for divergence up or down from here
            current_result_diverged = len(self.unique_stack_traces) > 1
            if current_result_diverged:
                bin_search_index.set_direction(-1)
                print("- diverged...")
            else:
                bin_search_index.set_direction(1)
                print("- converged...")

            """
            See if we've for sure narrowed in on the stack where the divergence first occurs.
            We'll only know for sure if we are testing adjacent indicies (e.g. frames 7 and 8) or
            we happen to be in the first frame (main). This can happen if the logic error really
            is in main and line numbers are embedded in the binary. Once we are at this condition,
            there are four possibilities:

            Case 1: Current round is a divergence - return the current result

            Case 2: Last iteration was a divergence, current round is a convergence - return the last result
            Case 3: Both the current and last iteration are converged - No divergence at! (return the last result)

            case 4: All passes are converged!
            """
            if abs(num_lines_to_keep - last_index) <= 1 or num_lines_to_keep == 1:
                # Case 1 & 4:
                if current_result_diverged or last_diverged_result == {}:
                    self.reportResults(self.unique_stack_traces, num_lines_to_keep)
                # Cases 2 & 3:
                else:
                    self.reportResults(last_diverged_result, last_diverged_index)
                return

            last_index = num_lines_to_keep
            if current_result_diverged:
                last_diverged_index = num_lines_to_keep
                last_diverged_result = self.unique_stack_traces.copy()

        raise Exception("Something went wrong with the search")

    def reportResults(self, results, diverged_stack_frame_num):
        """
        Reports the final results
        """
        print('\nUnique Stack Traces - frames_kept:', diverged_stack_frame_num)
        for trace, count in results.items():
            print(f'{"*"*80}\nCount: {len(count)}\n')
            if len(count) < 10:
                print("\n".join(count))
            print(f'\n{trace}')

    def compare_traces(self, trace1, trace2):
        """
        Return True if trace1 == trace2
        """
        # Drop Memory Addresses
        trace1 = self.memory_re.sub('0x...', trace1)
        trace2 = self.memory_re.sub('0x...', trace2)

        # Drop Frame Numbers
        trace1 = self.frame_num_re.sub('#00', trace1)
        trace2 = self.frame_num_re.sub('#00', trace2)

        if trace1 != trace2:
            return False

        return True

def main():
    """
    Parse arguments, gather hosts and launch SSH commands
    """
    parser = argparse.ArgumentParser(description="Normally you will run this script with your PBS job number AND the name of your executable (e.g. find_hung_processes.py 1234 my_app-opt)."
                                     "\nHowever, you can also pass in the name of the cache file containing traces from a previous run to process (e.g. find_hung_processes.py -f my_app-opt.1234.cache).")

    if '-f' not in sys.argv and '--file' not in sys.argv:
        parser.add_argument('pbs_job_num', type=int)
        parser.add_argument('application', type=str)

    parser.add_argument('-n', '--hosts', metavar='int', action='store', type=int, default=0,
                        help='The number of hosts to visit (Default: ALL)')
    parser.add_argument('-t', '--threads', metavar='int', type=int, default=12,
                        help='Number of threads used when remoting around gathering data. '
                        'Default: %(default)s')

    parser.add_argument('-f', '--file', metavar='str', action='store', type=str, default=None,
                        help='The existing cache file to process')
    parser.add_argument('-s', '--stacks', metavar='int', action='store', type=int, default=0,
                        help='The number of stack frames to keep and compare for '
                        'uniqueness (Default: ALL)')
    args = parser.parse_args()

    cache_filename = None
    if args.file == None:
        # Autogenerate a filename based on application name and job number
        cache_filename = f'{args.application}.{args.pbs_job_num}.cache'

        traces = generate_traces(args.pbs_job_num, args.application, args.hosts, args.threads)

        # Cache the restuls to a file
        with open(cache_filename, 'w', encoding='utf-8') as cache_file:
            for trace in traces:
                cache_file.write(f'{trace}{"*"*80}\n')
                cache_file.write('\n')
    else:
        cache_filename = args.file

    # See if we can open either the passed in file or the one we just generated
    if not os.path.exists(cache_filename):
        raise Exception(f"Unable to open trace cache file: cache_filename")

    # Process the traces to collapse them into unique stacks
    pt = ProcessTraces()
    pt.read_tracesfromfile(cache_filename)
    pt.process_traces(args.stacks)

if __name__ == '__main__':
    sys.exit(main())
