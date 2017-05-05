from subprocess import *
from util import *
from timeit import default_timer as clock

from Scheduler import Scheduler
from MooseObject import MooseObject
from tempfile import TemporaryFile

import os, sys, re, shutil, errno, platform, json

## Base class for providing routines common to queueing systems
class QueueManager(Scheduler):
    @staticmethod
    def validParams():
        params = Scheduler.validParams()
        params.addRequiredParam('command',                  '', "the tester command to run")
        params.addRequiredParam('mpi_procs',                '', "number of processors to request")
        params.addRequiredParam('template_script',  'template', "the template script used")
        params.addRequiredParam('job_name',                 '', "the path-friendly name of this test")
        params.addRequiredParam('working_dir',              '', "the working directory")

        params.addParam('no_copy_pattern', '.*\.sh',     "pattern of files not to copy")

        return params

    def __init__(self, harness, params):
        Scheduler.__init__(self, harness, params)

        # Used to store launched job information
        self.queue_file = harness.getQueueFile()
        self.queue_data = harness.getQueueData()

    # Get derived queue params
    def updateParams(self, tester):
        return

    # Get derived queue batch script requirements
    def prepareQueueScript(self, preq_list):
        return

    # Get the command we need to run
    def getQueueCommand(self):
        return

    # Get the command to run after all jobs have launched
    def getpostQueueCommand(self):
        return

    # Handle output generated from launching the job into the queue
    def handleQueueLaunch(self, tester, output):
        return

    # Handle output generated from getting information about the launched job
    def handleQueueStatus(self, tester, output):
        return

    # derived post command for this group of tests
    def postCommand(self):
        command = self.getpostQueueCommand()
        if command:
            runCommand(command)

    # Return path-friendly test name (remove special characters)
    def getJobName(self, tester):
        # A little hackish. But there is an instance where we do not
        # have a tester 'object' that we can to deal with. And that is
        # with the prereq test lists supplied by specs['prereq'].
        if type(tester) == type(str()):
            tester_text = tester
        else:
            tester_text = tester.specs['test_name']

        return ''.join(txt for txt in tester_text if txt.isalnum() or txt in ['_', '-'])

    # Return queue working directory
    def getWorkingDir(self, tester):
        return os.path.join(tester.specs['test_dir'], 'job_' + self.queue_file.name)

    # Return this test's queue ID if avaialble
    def getQueueID(self, tester):
        if self.getJobName(tester) in self.queue_data.keys():
            return self.queue_data[self.getJobName(tester)]['id']

    # Return location of queue launch script
    def getQueueScript(self, tester):
        return os.path.join(self.getWorkingDir(tester), self.queue_file.name + '-' + self.getJobName(tester) + '.sh')

    # Return json queued test information
    def getQueueData(self, tester):
        if self.getJobName(tester) in self.queue_data:
            return self.queue_data[self.getJobName(tester)]

    # Record launched job information
    def updateQueueFile(self, tester, queue_id, skipped=False, exit_code=None):
        if skipped:
            queue_id = None
        # Update the queue_data global so that it can be saved/retreived to/from the json file.
        self.queue_data[self.getJobName(tester)] = { 'job_name'     : self.getJobName(tester),
                                                     'input_name'   : self.options.input_file_name,
                                                     'test_dir'     : self.getWorkingDir(tester),
                                                     'skipped'      : skipped,
                                                     'status'       : tester.getStatusMessage(),
                                                     'exit_code'    : exit_code,
                                                     'id'           : queue_id }

    # Verify we have this test in our records if we are processing the queue
    # If it was skipped before, skip it again (displaying caveats)

    # If it was skipped with a silent attribute (--re= options etc) do not save this
    # information into the queue file. This will have the effect of being 'silent'
    def canLaunch(self, tester, command, checks, test_list):
        # We are processing a previous run
        if self.options.processingQueue:
            if self.getJobName(tester) in self.queue_data.keys():
                if self.queue_data[self.getJobName(tester)]['skipped']:
                    tester.setStatus(self.queue_data[self.getJobName(tester)]['status'], tester.bucket_skip)
                else:
                    return True
        # We are launching jobs normally
        else:
            should_run = tester.checkRunnableBase(self.options, checks, test_list)
            if should_run:
                return True
            else:
                # Do not save silent|deleted skipped tests to the queue file
                if tester.getStatus() != tester.bucket_silent and tester.getStatus() != tester.bucket_deleted:
                    self.updateQueueFile(tester, 0, True)
        return False

    # Augment base queue paramaters with tester params
    def updateParamsBase(self, tester, command):
        # Set processor count
        self.specs['mpi_procs'] = tester.getProcs(self.options)

        # Set job name
        self.specs['job_name'] = self.getJobName(tester)

        # Set working directory
        self.specs['working_dir'] = self.getWorkingDir(tester)

        #### create a set for copy and nocopy so its easier to work with
        no_copy_files = set([])
        copy_files = set([])

        if tester.specs.isValid('no_copy_files'):
            no_copy_files.update(tester.specs['no_copy_files'])
        if tester.specs.isValid('copy_files'):
            copy_files.update(tester.specs['copy_files'])
        if tester.specs.isValid('gold_dir'):
            copy_files.update([tester.specs['gold_dir']])

        #### convert the copy and nocopy sets to flat lists
        self.specs['copy_files'] = ' '.join(copy_files)
        self.specs['no_copy'] = ' '.join(no_copy_files)

        # The command the tester needs to run within the queue system
        self.specs['command'] = command

        # ask the derived classes to update any specific queue specs
        self.updateParams(tester)

        return

    # Create the queue launch script
    def prepareQueueScriptBase(self, tester, preq_list):
        # Augment derived queue script requirements
        self.prepareQueueScript(tester, preq_list)

        f = open(self.specs['template_script'], 'r')
        content = f.read()
        f.close()

        params = self.specs

        f = open(self.getQueueScript(tester), 'w')

        # Do all of the replacements for the valid parameters
        for param in params.valid_keys():
            if param in params.substitute:
                params[param] = params.substitute[param].replace(param.upper(), params[param])
            content = content.replace('<' + param.upper() + '>', str(params[param]))

        # Make sure we strip out any string substitution parameters that were not supplied
        for param in params.substitute_keys():
            if not params.isValid(param):
                content = content.replace('<' + param.upper() + '>', '')

        f.write(content)
        f.close()

    # Called from the current directory to copy files
    def copyFiles(self, tester):
        params = self.specs

        # Create regexp object of no_copy_pattern
        if params.isValid('no_copy_pattern'):
            # Match no_copy_pattern value
            pattern = re.compile(params['no_copy_pattern'])
        else:
            # Match nothing if not set. Better way?
            pattern = re.compile(r'')

        # Create the job directory
        if not os.path.exists(self.getWorkingDir(tester)):
            try:
                os.makedirs(self.getWorkingDir(tester))
            except OSError, ex:
                if ex.errno == errno.EEXIST: pass
                else: raise

        # Copy files (unless they are listed in "no_copy")
        current_pwd = os.getcwd()
        os.chdir(self.getWorkingDir(tester))
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
                        else: raise Exception()

        # Create symlinks (do last, to allow hard copy above to be default)
        if tester.specs.isValid('link_files'):
            for file in tester.specs['link_files']:
                if os.path.exists('../' + file):
                    try:
                        os.symlink('../' + file, file)
                    except OSError, ex:
                        if ex.errno == errno.EEXIST: pass
                        else: raise Exception()

        # return to the previous directory
        os.chdir(current_pwd)

    ## run the command asynchronously and call testharness.testOutputAndFinish when complete
    ## Note: slot_check should be False for launching jobs to a third party queueing system
    def run(self, tester, command, recurse=True, slot_check=False):
        # First see if any of the queued jobs can be run but only if recursion is allowed on this run
        if recurse:
            self.startReadyJobs(slot_check)

        # The command the tester needs us to launch
        tester_command = command

        # Augment Queue params with Tester params
        self.updateParamsBase(tester, tester_command)

        # Get the derived queueing command the queueing system needs us to launch
        command = self.getQueueCommand(tester)

        # Handle unsatisfied prereq tests if we are trying to launch jobs.
        # If we are trying to process results instead, skip this check.
        if not self.options.processingQueue:
            if self.unsatisfiedPrereqs(tester):
                self.queue.append([tester, tester_command, os.getcwd()])
                return

        # If we are trying to process results instead, skip this check.
        if not self.options.processingQueue:
            # This test can now be launched because the prereq test(s) have been satisfied above
            prereq_job_ids = []
            if tester.specs['prereq'] != [] and not self.options.processingQueue:
                for prereq_test in tester.specs['prereq']:
                    path_friendly = self.getJobName(prereq_test)
                    prereq_job_ids.append(self.queue_data[path_friendly]['id'])

            # Create and copy all the files/directories this test requires
            self.copyFiles(tester)

            # Call the derived method to build the queue batch script
            self.prepareQueueScriptBase(tester, prereq_job_ids)

        # Move into the working_dir
        current_pwd = os.getcwd()
        os.chdir(self.getWorkingDir(tester))

        # We're good up to this point? Then launch the job!
        self.launchJob(tester, command, slot_check)

        # Return to the previous directory
        os.chdir(current_pwd)

    ## Return control to the test harness by finalizing the test output and calling the callback
    def returnToTestHarness(self, job_index):
        (p, command, tester, time, f, slots) = self.jobs[job_index]
        log( 'Command %d done:    %s' % (job_index, command) )
        output = 'Working Directory: ' + self.getWorkingDir(tester) + '\nRunning command: ' + command + '\n'

        if p.poll() == None: # process has not completed, it timed out
            output += '\n' + "#"*80 + '\nProcess terminated by test harness. Max time exceeded (' + str(tester.specs['max_time']) + ' seconds)\n' + "#"*80 + '\n'
            f.close()
            tester.setStatus('TIMEOUT', tester.bucket_fail)
            if platform.system() == "Windows":
                p.terminate()
            else:
                pgid = os.getpgid(p.pid)
                os.killpg(pgid, SIGTERM)

            self.harness.testOutputAndFinish(tester, Scheduler.TIMEOUT, output, time, clock())
        else:

            # Get stdout from executed command
            queue_output = self.readOutput(f)
            f.close()

            # determine tester statuses depending on mode (launching or checking status)
            if self.options.processingQueue:
                # the test was already launched
                output += self.handleQueueStatus(tester, queue_output)
            else:
                # the test needed to be launched
                output += self.handleQueueLaunch(tester, queue_output)

            # TestOutputAndFinish
            # Test is finished regardless of a passing status (so add to results table)
            if tester.didFail() or tester.didPass():
                self.finished_jobs.add(tester.specs['test_name'])
                self.harness.testOutputAndFinish(tester, p.returncode, output, time, clock())

            # Handle TestResult
            # Test can still be pending in the queue (but we need to add these results to the table)
            elif self.options.processingQueue and tester.getStatus() == tester.bucket_pending:
                self.skipped_jobs.add(tester.specs['test_name'])
                self.harness.handleTestResult(tester, output, tester.getStatusMessage())

            # Handle TestStatus
            # Test has been launched by qsub (we do not want to include this in final results table)
            else:
                self.finished_jobs.add(tester.specs['test_name'])
                #self.harness.handleTestStatus(tester, output)
                self.harness.handleTestResult(tester, output, tester.getStatusMessage())

        self.jobs[job_index] = None
        self.slots_in_use = self.slots_in_use - slots

## Static logging string for debugging
LOG = []
LOG_ON = False
def log(msg):
    if LOG_ON:
        LOG.append(msg)
        print msg
