from subprocess import *
from timeit import default_timer as clock

from Scheduler import Scheduler
from MooseObject import MooseObject
from tempfile import TemporaryFile

import os, sys, re, shutil, errno, platform

## Base class for providing routines common to queueing systems
class QueueManager(Scheduler):
    @staticmethod
    def validParams():
        params = Scheduler.validParams()
        params.addRequiredParam('scheduler',    'QueueManager', "The name of this scheduler")
        params.addRequiredParam('job_name',                 '', "path friendly name of this job")
        params.addRequiredParam('command',                  '', "the tester command to run")
        params.addRequiredParam('mpi_procs',                '', "number of processors to request")
        params.addRequiredParam('working_dir',              '', "the working directory to change to")

        params.addParam('copy_files',            '',     "list of files to copy")
        params.addParam('no_copy_files',         '',     "list of files not to copy")
        params.addParam('no_copy_pattern', '.*\.sh',     "pattern of files not to copy")
        params.addParam('pre_script',            '',     "location of bash script to execute before launching command (useful to load modules, set environment variables etc)")

        return params

    def __init__(self, harness, params):
        Scheduler.__init__(self, harness, params)

        # Used to store launched job information
        self.launched_jobs = {}
        self.queue_file = harness.getQueueFile()

    # return this test's queue ID if avaialble
    def getJobID(self):
        if self.specs['job_name'] in self.queue_data.keys():
            return self.queue_data[self.specs['job_name']]['ID']
        return None

    # Update derived Queue params with tester params
    def updateParams(self, tester):
        return

    # Augment base queue paramaters with tester params
    # Do not override this method. Use updateParams instead
    def updateParamsBase(self, tester, queue_command):
        # Set a path friendly test name
        self.specs['job_name'] = tester.specs['test_name'].replace('/', '_')

        # Set processor count
        self.specs['mpi_procs'] = tester.getProcs(self.options)

        #### create a set for copy and nocopy so its easier to work with
        no_copy_files = set([self.options.input_file_name])
        copy_files = set([])

        if tester.specs.isValid('no_copy_files'):
            no_copy_files.update(tester.specs['no_copy_files'])
        if tester.specs.isValid('copy_files'):
            copy_files.update(tester.specs['copy_files'])
        copy_files.update([tester.specs['gold_dir']])

        #### convert the copy and nocopy sets to flat lists so PBSJob can work with them
        self.specs['copy_files'] = ' '.join(copy_files)
        self.specs['no_copy'] = ' '.join(no_copy_files)

        # Set queue working directory to change to when queueing system runs queue_command
        self.specs['working_dir'] = os.path.join(tester.specs['test_dir'], 'job_' + self.queue_file.name)

        # The command the tester needs to run within the queue system
        self.specs['command'] = queue_command

        # Save the queue_script location for easy use
        self.queue_script = os.path.join(self.specs['working_dir'], self.queue_file.name + '-' + self.specs['job_name'] + '.sh')

        # ask the derived classes to update their specific queue specs
        self.updateParams(tester)

        return

    # Called from the current directory to copy files (usually from the parent)
    def copyFiles(self):
        params = self.specs

        # Create regexp object of no_copy_pattern
        if params.isValid('no_copy_pattern'):
            # Match no_copy_pattern value
            pattern = re.compile(params['no_copy_pattern'])
        else:
            # Match nothing if not set. Better way?
            pattern = re.compile(r'')

        # Copy files (unless they are listed in "no_copy")
        for file in os.listdir('../'):
            if os.path.isfile('../' + file) and file != self.options.input_file_name and \
               (not params.isValid('no_copy') or file not in params['no_copy']) and \
               (not params.isValid('no_copy_pattern') or pattern.match(file) is None) and \
               not os.path.exists(file) and \
               os.path.splitext(file)[1] != '':
                shutil.copy('../' + file, '.')

        # Copy directories
        if params.isValid('copy_files'):
            for file in params['copy_files'].split():
                if os.path.isfile('../' + file):
                    if not os.path.exists(file):
                        shutil.copy('../' + file, '.')
                elif os.path.isdir('../' + file):
                    try:
                        shutil.copytree('../' + file, file)
                    except OSError, ex:
                        if ex.errno == errno.EEXIST: pass
                        else: raise

    ## run the command asynchronously and call testharness.testOutputAndFinish when complete
    def run(self, tester, command, recurse=True, slot_check=True):
        # First see if any of the queued jobs can be run but only if recursion is allowed on this run
        if recurse:
            self.startReadyJobs(slot_check)

        # The command the tester needs us to launch
        queue_command = command

        # Augment Queue params with Tester params
        self.updateParamsBase(tester, queue_command)

        # The command the queueing system needs us to launch
        command = self.getQueueCommand(tester)

        # Handle unsatisfied prereq tests if we are trying to launch jobs
        # If we are trying to process results instead, skip this check
        if not self.options.processingQueue:
            if self.unsatisfiedPrereqs(tester):
                self.queue.append([tester, command, os.getcwd()])
                return

        # This test can now be launched because the prereq test(s) have been launched. This
        # means we can now fill in the PBS job id dependency portion of the launch script.
        prereq_job_ids = []
        if tester.specs['prereq'] != [] and not self.options.processingQueue:
            file = os.path.join(tester.specs['test_dir'], self.options.queue_file.name + '-' + tester.specs['job_name'] + '.sh')
            for prereq_test in tester.specs['prereq']:
                prereq_job_ids.append(self.launched_jobs[prereq_test])

        # Create Queue launch script
        if not self.options.processingQueue:
            self.prepareQueueScript(prereq_job_ids)


    ## Return control to the test harness by finalizing the test output and calling the callback
    def returnToTestHarness(self, job_index):
        return
