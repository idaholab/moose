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

import string
import sys
import os
import re
import subprocess
import argparse
from tempfile import TemporaryFile
from time import sleep
from multiprocessing.pool import ThreadPool
import threading # for thread locking and thread timers
import multiprocessing # for timeouts

##################################################################
# Modify the following variable(s) for your cluster or use one of the versions below
### FISSION
#node_name_pattern = re.compile('(fission-\d{4})')
#pstack_binary = 'pstack'
### BECHLER
#node_name_pattern = re.compile('(b\d{2})')
#pstack_binary = 'pstack'
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
        if self.get_process().poll():
            self.__stdout_file.seek(0)
            return self.__stdout_file.read().decode('utf-8')
        return None
    def get_stderr(self):
        """
        Return stderr if process has completed
        """
        if self.get_process().poll():
            self.__stderr_file.seek(0)
            return self.__stderr_file.read().decode('utf-8')
        return None
    def run(self, e_timeout=None):
        """
        Execute command and wait for return code
        """
        with subprocess.Popen(self.__command,
                              stdout=self.__stdout_file,
                              stderr=self.__stderr_file,
                              shell=True,
                              encoding='utf-8') as self.__process:
            self.__process.wait(timeout=e_timeout)

class Scheduler:
    """
    A simple threading object handler.

    Syntax:
      jobs = [[job], [job]]
      runner = Scheduler()
      runner.schedule_jobs(jobs)

    .schedule_jobs() blocks until all jobs launch.
    .wait_finish() blocks until all jobs finish.
    .get_finished() returns
    """
    def __init__(self, max_slots=6, e_timeout=None):
        self.max_slots = int(max_slots)
        self.worker_pool = ThreadPool(processes=self.max_slots)
        self.thread_lock = threading.Lock()
        self.job_queue = set([])
        self.active = set([])
        self.finished = set([])
        self.__timeout = e_timeout

    def wait_finish(self):
        """
        block until all submitted jobs are finished
        """
        while self.job_queue or self.active:
            sleep(0.5)
        self.worker_pool.close()
        self.worker_pool.join()

    def schedule_jobs(self, jobs):
        """
        Begin launching jobs by providing a list of lists with
        shell commands to run.
        """
        with self.thread_lock:
            for a_job in jobs:
                o_job = Job(a_job)
                self.job_queue.add(o_job)
        self.queue_jobs(self.job_queue)

    def queue_jobs(self, jobs):
        """
        Add all jobs to the thread pool
        """
        for o_job in jobs:
            self.worker_pool.apply_async(self.launch_job, (o_job,))

    def reserve_allocation(self, o_job):
        """
        Return bool if enough resources exist to launch job
        """
        with self.thread_lock:
            if len(self.active) < self.max_slots:
                self.active.add(o_job)
                self.job_queue.remove(o_job)
                return True
        return False

    def launch_job(self, o_job):
        """
        Run subprocess using provided command list
        """
        try:
            if self.reserve_allocation(o_job):
                o_job.run(timeout=self.__timeout)
                with self.thread_lock:
                    self.active.remove(o_job)
                    self.finished.add(o_job)
            else:
                sleep(.1)
                self.queue_jobs([o_job])
        except KeyboardInterrupt:
            sys.exit(1)


def run_command(command, timeout=10):
    """
    Run a command and return stdout.

    Quits if command does not return within 10 seconds (default), or
    if command results in non-zero return code.
    """
    with subprocess.Popen(f'{command}',
                          shell=True,
                          stdout=subprocess.PIPE) as run_proc:
        try:
            run_proc.wait(timeout=timeout)
            (s_stdout, s_stderr) = run_proc.communicate(timeout=timeout)
            if run_proc.poll():
                print(f'Error running {command}\n{s_stderr.decode("utf-8")}')
                sys.exit(run_proc.poll())
            return s_stdout.decode('utf-8')
        except subprocess.TimeoutExpired:
            print(f'Timeout after {timeout} seconds waiting for {command}')
            sys.exit(1)
        except KeyboardInterrupt:
            sys.exit(1)

def get_sshoutput(application_name, host):
    """
    Generate and return a valid SSH remote execution command
    """
    ps_grep = f'ps -e | grep {application_name}'
    awk_print = r'awk \'{print \$1}\''
    xargs_echo = f'xargs -I \'{{}}\' sh -c \'echo Host: {host} PID: {{}}; ' \
                 f'{PSTACK_BINARY} {{}}; printf \'*%.0s\' {{1..80}}; echo\' '
    command = f'ssh {host} "{ps_grep} | {awk_print} | {xargs_echo}"'
    return run_command(command)

def generate_traces(job_num, application_name, num_hosts):
    """
    Generate a temporary file with traces and return
    formated results
    """
    # The lists of hosts
    hosts = []
    # The array of jobs
    jobs = []
    # The machine name should go here!

    host_strs = node_name_pattern.findall(run_command(f'qstat -n {job_num}'))
    for i in host_strs:
        hosts.append(i)

    # Launch all the jobs
    if num_hosts == 0:
        num_hosts = len(hosts)

    for i in range(len(hosts)):
        if i >= num_hosts:
            continue


        temp_f = TemporaryFile()
        ssh_proc = subprocess.Popen(command, stdout=temp_f, close_fds=False, shell=True)
        jobs.append((ssh_proc, temp_f))

    # Now process the output from each of the jobs
    traces = []
    for (ssh_proc, temp_f) in jobs:
        ssh_proc.wait()
        temp_f.seek(0)
        output = temp_f.read()
        temp_f.close()

        # strip blank lines
        output = os.linesep.join([s for s in output.splitlines() if s])

        traces.extend(split_traces(output))

    return traces

def read_tracesfromfile(filename):
    """
    Read tracefile, and return a list of traces
    """
    with open(filename) as t_file:
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
    last_lines_regex = re.compile(rf'(?:.*\n){{str(num_lines_to_keep)}}\Z', re.M)
    host_regex = re.compile('^(Host.*)', re.M)

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
            if compare_traces(trace, bt):
                unique = bt

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
    for i in range(len(lines1)):
        line1 = lines1[i].split()[2:]
        line2 = lines2[i].split()[2:]

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
    parser.add_argument('PBS job number', type=int)
    parser.add_argument('Application', type=string)
    parser.add_argument('-s', '--stacks', action='store', dest='stacks', type='int',
                      default=0, help='The number of stack frames to keep and compare '
                      'for uniqueness (Default: ALL)')
    parser.add_argument('-n', '--hosts', action='store', dest='hosts', type='int',
                      default=0, help='The number of hosts to visit (Default: ALL)')
    parser.add_argument('-f', '--force', action='store_true', dest='force',
                      default=False, help='Whether or not to force a regen if a cache '
                      'file exists')
    args = parser.parse_args()

    if len(args) != 2:
        parser.print_help()
        sys.exit(1)

    # The PBS job number and the application should be passed on the command line
    # Additionally, an optional argument of the number of frames to keep (compare) may be passed
    job_num = args[0]
    application = args[1]
    num_to_keep = args.stacks
    num_hosts = args.hosts

    # first see if there is a cache file available
    cache_filename = f'{application}.{job_num}.cache'

    traces = []
    if not os.path.exists(cache_filename) or args.force:
        traces = generate_traces(job_num, application, args.hosts)

        # Cache the restuls to a file
        cache_file = open(cache_filename, 'w')
        for trace in traces:
            cache_file.write(f'{trace}{"*"*80}\n')
        cache_file.write('\n')
        cache_file.close()

    # Process the traces to collapse them into unique stacks
    traces = read_tracesfromfile(cache_filename)
    unique_stack_traces = process_traces(traces, num_to_keep)

    print('Unique Stack Traces')
    for trace, count in unique_stack_traces.iteritems():
        print(f'{"*"*80}\nCount: {len(count)}\n')
        if len(count) < 10:
            print("\n".join(count))
        print(f'\n{trace}')


if __name__ == '__main__':
    sys.exit(main())
