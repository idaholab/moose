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
        params.addRequiredParam('template_script',          '', "the template script used")
        params.addRequiredParam('command',                  '', "the tester command to run")
        params.addRequiredParam('mpi_procs',                '', "number of processors to request")
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
    def prepareQueueScript(self, template_queue, tester, preq_list):
        return

    # Get the command we need to run
    def getQueueCommand(self, tester):
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

    # Return a list of tuples of test files to run
    # This needs to return an absolute path to a directory containing 'tests' file
    # along with the 'test' file:
    # [(dirpath, tests), (dirpath, tests)]
    def getTests(self):
        test_list = []
        if self.queue_data:
            for tests in self.queue_data.keys():
                test_dir = os.path.dirname(self.queue_data[tests]['test_dir'])
                test_file = self.queue_data[tests]['input_name']
                test_list.append((test_dir, test_file))
        return test_list

    # Go Again!
    def goAgain(self):
        if not self.options.checkStatus:
            print 'Jobs released, begin checking status...\n'

            # Clear the scheduler queue in preperation to go again
            self.clearAndInitializeJobs()

            # Set the checkStatus
            self.options.checkStatus = True

            return True

    # derived post command after launching tests
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

    # Update queue_data with launched job information
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

    # The Scheduler has updated a tester's status. If the test is skipped, add it
    # to the queue file, so it gets printed again during a checkStatus
    def notifySchedulers(self, tester):
        if tester.isSkipped():
            self.updateQueueFile(tester, 0, True)
        return

    # canLaunch is the QueueManager's way of providing checkRunnable due to the multitude of
    # modes the TestHarness can operate in.
    def canLaunch(self, tester, checks, test_list):
        # We are processing a previous run
        if self.options.checkStatus:

            # Only launch a test if not launched yet, and if it exists in our queue_file
            if tester.specs['test_name'] not in self.launched_jobs and tester.specs['test_name'] not in self.skipped_jobs \
               and self.getJobName(tester) in self.queue_data.keys():

                # This test, while previously skipped, should have its skipped caveats re-displayed (unless the
                # user is only interested in failed tests)
                if self.queue_data[self.getJobName(tester)]['skipped'] and not self.options.failed_tests:
                    tester.setStatus(self.queue_data[self.getJobName(tester)]['status'], tester.bucket_skip)

                # Continue to support checkRunnable (--re options etc)
                else:
                    # TODO: this might stop a test from running when in fact did run previously under some strange
                    # circumstances. Need to test this.
                    return tester.checkRunnableBase(self.options, checks, test_list)

        # We are launching jobs normally, ask checkRunnable if we can run this test
        else:
            should_run = tester.checkRunnableBase(self.options, checks, test_list)
            if should_run:
                return True

            # Do not launch this test. However, save the skipped caveat if the test status is not of the silent variety
            else:
                if tester.getStatus() != tester.bucket_silent and tester.getStatus() != tester.bucket_deleted:
                    self.updateQueueFile(tester, 0, True)
        return False

    # Augment base queue paramaters with tester params
    def updateParamsBase(self, tester, command):
        # Initialize our template_dictionary
        template_queue = {}

        # Loops through params and inject them into our tempalte_dictionary
        for param in self.specs.keys():
            template_queue[param] = self.specs[param]

        # Set CPU request count
        template_queue['mpi_procs'] = tester.getProcs(self.options)

        # Set job name
        template_queue['job_name'] = self.getJobName(tester)

        # Set working directory
        template_queue['working_dir'] = self.getWorkingDir(tester)

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
        template_queue['copy_files'] = copy_files
        template_queue['no_copy'] = no_copy_files

        # The command the tester needs to run within the queue system
        template_queue['command'] = command

        # ask the derived classes to update any specific queue specs
        template_queue = self.updateParams(template_queue, tester)

        return template_queue

    # Create the queue launch script
    def prepareQueueScriptBase(self, template_queue, tester, preq_list):
        # Augment derived queue script requirements
        template_queue = self.prepareQueueScript(template_queue, tester, preq_list)

        f = open(template_queue['template_script'], 'r')
        content = f.read()
        f.close()

        f = open(self.getQueueScript(tester), 'w')

        # Do all of the replacements for the valid parameters
        for key in template_queue.keys():
            if key.upper() in content:
                content = content.replace('<' + key.upper() + '>', str(template_queue[key]))

        # Make sure we strip out any string substitution parameters that were not supplied
        for key in template_queue.keys():
            if key.upper() not in content:
                content = content.replace('<' + key.upper() + '>', '')

        f.write(content)
        f.close()

    # Called from the current directory to copy files
    def copyFiles(self, template_queue, tester):

        # Create regexp object of no_copy_pattern
        if 'no_copy_pattern' in template_queue.keys():
            # Match no_copy_pattern value
            pattern = re.compile(template_queue['no_copy_pattern'])
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
               (not tester.specs.isValid('no_copy') or file not in tester.specs['no_copy']) and \
               (not 'no_copy_pattern' in template_queue.keys() or pattern.match(file) is None) and \
               not os.path.exists(file) and \
               os.path.splitext(file)[1] != '':
                shutil.copy('../' + file, '.')

        # Copy directories
        if 'copy_files' in template_queue.keys():
            for file in template_queue['copy_files']:
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

        # Build template_queue params based on Tester params
        template_queue = self.updateParamsBase(tester, tester_command)

        # Get the derived queueing command the queueing system needs us to launch
        command = self.getQueueCommand(tester)

        # Handle unsatisfied prereq tests if we are trying to launch jobs.
        # If we are trying to process results instead, skip this check.
        if not self.options.checkStatus:
            if self.unsatisfiedPrereqs(tester):
                self.queue.append([tester, tester_command, os.getcwd()])
                return

        # If we are trying to process results instead, skip this check.
        if not self.options.checkStatus:
            # This test can now be launched because the prereq test(s) have been satisfied above
            prereq_job_ids = []
            if tester.specs['prereq'] != [] and not self.options.checkStatus:
                for prereq_test in tester.specs['prereq']:
                    path_friendly = self.getJobName(prereq_test)
                    prereq_job_ids.append(self.queue_data[path_friendly]['id'])

            # Create and copy all the files/directories this test requires
            self.copyFiles(template_queue, tester)

            # Call the derived method to build the queue batch script
            self.prepareQueueScriptBase(template_queue, tester, prereq_job_ids)

        # Move into the working_dir
        current_pwd = os.getcwd()
        os.chdir(self.getWorkingDir(tester))

        # We're good up to this point? Then launch the job!
        self.launchJob(tester, command, slot_check)

        # Return to the previous directory
        os.chdir(current_pwd)

    ## Return control to the test harness by finalizing the test output and calling the callback
    def returnToTestHarness(self, job_index, output):
        (p, command, tester, time, f, slots) = self.jobs[job_index]

        # Get stdout from executed command
        queue_output = self.readOutput(f)

        # determine tester statuses depending on mode (launching or checking status)
        if self.options.checkStatus:
            # the test was already launched
            output += self.handleQueueStatus(tester, queue_output)
        else:
            # the test needed to be launched
            output += self.handleQueueLaunch(tester, queue_output)

        # Launching Jobs
        # When launching jobs we need to _not_ add results to the 'final results' table
        # but only if they didn't fail during launch
        if not self.options.checkStatus and tester.isPending():
            self.finished_jobs.add(tester.specs['test_name'])
            self.harness.handleTestResult(tester, output, tester.getStatusMessage(), add_to_table=False)

        # Launching Job or Checking Status and the test failed
        elif tester.didFail():
            self.finished_jobs.add(tester.specs['test_name'])
            self.harness.testOutputAndFinish(tester, p.returncode, output, time, clock())

        # Checking Status and the test is still pending
        elif self.options.checkStatus and tester.isPending():
            self.skipped_jobs.add(tester.specs['test_name'])
            self.harness.testOutputAndFinish(tester, p.returncode, output, time, clock())

        # Checking Status and the test has finished
        elif self.options.checkStatus and not tester.isPending():
            self.finished_jobs.add(tester.specs['test_name'])
            self.harness.testOutputAndFinish(tester, p.returncode, output, time, clock())
