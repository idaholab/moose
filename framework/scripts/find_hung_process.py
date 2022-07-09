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
"""

import sys
import os
import re
import subprocess
import argparse
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
        traces.extend(split_traces(output))
    return traces

def read_tracesfromfile(filename):
    """
    Read tracefile, and return a list of traces
    """
    with open(filename, encoding='utf-8') as t_file:
        data = t_file.read()
        return split_traces(data)

def split_traces(trace_string):
    """
    Return list of traces based on regular expression:

    Only keep lines beginning with a #
    throw_away = re.compile("^[^#].*", re.M)
    traces = [throw_away.sub("", trace) for trace in traces]
    """
    trace_regex = re.compile(r'^\**\n', re.M)
    traces = trace_regex.split(trace_string)
    return traces

def process_traces(traces, num_lines_to_keep):
    """
    Process the individual traces
    """
    unique_stack_traces = {}
    last_lines_regex = re.compile(fr'(?:.*\n){{{num_lines_to_keep}}}\Z', re.M)
    host_regex = re.compile('^(Host.*)', re.M)

    for trace in traces:
        if len(trace) == 0:
            continue

        # Grab the host and PID
        tmp_trace = host_regex.search(trace)
        if tmp_trace:
            host_pid = tmp_trace.group(1)

        # If the user requested to save only the last few lines, do that here
        if num_lines_to_keep:
            tmp_trace = last_lines_regex.search(trace)
            if tmp_trace:
                trace = tmp_trace.group(0)

        unique = ''
        for back_trace in unique_stack_traces:
            if compare_traces(trace, back_trace):
                unique = back_trace

        if unique == '':
            unique_stack_traces[trace] = [host_pid]
        else:
            unique_stack_traces[unique].append(host_pid)

    return unique_stack_traces

def compare_traces(trace1, trace2):
    """
    Return bool if trace1 = trace2
    """
    lines1 = trace1.split('\n')
    lines2 = trace2.split('\n')

    if len(lines1) != len(lines2):
        return False

    # Only compare the stack trace part - not the memory addresses
    # Note this subroutine may need tweaking if the stack trace is different
    # on the current machine
    memory_re = re.compile('0x[0-9a-f]*')
    for a_line in lines1:
        line1 = a_line.split()[2:]
        line2 = a_line.split()[2:]
        # Let's strip out all the memory addresses too
        line1 = [memory_re.sub('0x...', line) for line in line1]
        line2 = [memory_re.sub('0x...', line) for line in line2]
        if line1 != line2:
            return False
    return True

def main():
    """
    Parse arguments, gather hosts and launch SSH commands
    """
    parser = argparse.ArgumentParser(description='Usage: %prog [options] <PBS Job num> '
                                                 '<Application>')
    parser.add_argument('pbs_job_num', type=int)
    parser.add_argument('application', type=str)
    parser.add_argument('-s', '--stacks', metavar='int', action='store', type=int, default=0,
                       help='The number of stack frames to keep and compare for '
                       'uniqueness (Default: ALL)')
    parser.add_argument('-n', '--hosts', metavar='int', action='store', type=int, default=0,
                       help='The number of hosts to visit (Default: ALL)')
    parser.add_argument('-f', '--force', action='store_true', default=False,
                       help='Whether or not to force a regen if a cache file exists')
    parser.add_argument('-t', '--threads', metavar='int', type=int, default=12,
                       help='Number of threads used when remoting around gathering data. '
                       'Default: %(default)s')
    args = parser.parse_args()

    # first see if there is a cache file available
    cache_filename = f'{args.application}.{args.pbs_job_num}.cache'

    traces = []
    if not os.path.exists(cache_filename) or args.force:
        traces = generate_traces(args.pbs_job_num, args.application, args.hosts, args.threads)

        # Cache the restuls to a file
        with open(cache_filename, 'w', encoding='utf-8') as cache_file:
            for trace in traces:
                cache_file.write(f'{trace}{"*"*80}\n')
                cache_file.write('\n')

    # Process the traces to collapse them into unique stacks
    traces = read_tracesfromfile(cache_filename)
    unique_stack_traces = process_traces(traces, args.stacks)

    print('Unique Stack Traces')
    for trace, count in unique_stack_traces.items():
        print(f'{"*"*80}\nCount: {len(count)}\n')
        if len(count) < 10:
            print("\n".join(count))
        print(f'\n{trace}')


if __name__ == '__main__':
    sys.exit(main())
