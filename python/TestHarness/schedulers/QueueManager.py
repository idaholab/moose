from subprocess import *
from util import *
from collections import namedtuple
from timeit import default_timer as clock

from Scheduler import Scheduler
from MooseObject import MooseObject
from tempfile import TemporaryFile

import os, sys, re, shutil, errno

# Base class for providing an interface to an external scheduler
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

    # convert a dictionary_bucket status to a tester_bucket status
    def createStatusBucket(self, json_status):
        # Create a status_bucket according to what is stored in the json file
        test_status = namedtuple('status_bucket', json_status)
        return test_status(status=json_status['status'], color=json_status['color'])

    # Update tester status in queue_data
    def updateTesterStatus(self, tester):
        self.putData(self.getJobName(tester),
                     job_name       = self.getJobName(tester),
                     skipped        = tester.isSkipped(),
                     is_pending     = tester.isPending(),
                     is_silent      = tester.isSilent(),
                     is_deleted     = tester.isDeleted(),
                     did_pass       = tester.didPass(),
                     did_fail       = tester.didFail(),
                     did_diff       = tester.didDiff(),
                     test_dir       = self.getWorkingDir(tester),
                     test_name      = tester.getTestName(),
                     input_name     = self.options.input_file_name,
                     status_message = tester.getStatusMessage(),
                     status_bucket  = dict(tester.getStatus()._asdict()))

    # Return a set of dependents for a tester
    def getDependents(self, tester):
        r = ReverseReachability()
        # To save processes we are only interested in obtaining tests related to this tester by matching
        # test_dir (because we have to iterate over the entire queue_data dictionary) I do not like the
        # way I am doing this. But I have no access to the list of tester objects (QueueManager only
        # receives one tester at a time). Better way?
        for job_name, values in self.queue_data.iteritems():
            if values['test_dir'] == self.getWorkingDir(tester):
                if 'prereq_tests' in values.keys() and 'test_name' in values.keys():
                    r.insertDependency(values['test_name'], values['prereq_tests'])

                elif 'test_name' in values.keys():
                    r.insertDependency(values['test_name'], [])

        return r.getReverseReachabilitySets()

    # Update the dependencies stored in the queue_data
    def updateDependencies(self, tester):
        sorted_values = self.getDependents(tester)
        for job_name, deps in sorted_values.iteritems():
            self.putData(self.getJobName(job_name), depends_on_me=list(deps))

    # Get specified data from queue_data
    def getData(self, job, **kwargs):
        tmp_dict = {}
        if job in self.queue_data.keys():
            for kkey in kwargs.keys():
                try:
                    tmp_dict[kkey] = self.queue_data[job][kkey]
                except KeyError: # Requested key not available
                    tmp_dict[kkey] = None
        return tmp_dict

    # Set queue_data (this overwrites attributes)
    def putData(self, job, **kwargs):
        for key, value in kwargs.iteritems():
            if job in self.queue_data.keys():
                self.queue_data[job][key] = value
            else:
                self.queue_data[job] = {key : value}

    # Update queue_data (appends specified values)
    def updateData(self, job, **kwargs):
        data = self.getData(job, **kwargs)
        data_copy = data.copy()
        for key, value in kwargs.iteritems():
            tmp_dict = { key : value }

            # Nothing to update, treat as new data (putData)
            if data[key] != None:

                # There is current data we need to append to
                if type(data[key]) == type([]) and type(value) == type([]):
                    data_copy[key].extend(value)
                    self.putData(job, **data_copy)

                # request can not be satisfied (updateData only supports updating list attributes)
                else:
                    raise Exception('updateData only supports updating list types')
            else:
                self.putData(job, **tmp_dict)

    # Return a list of tuples containing abspath of test_dir and corresponding test file and test name from
    # the queue_data
    def getTests(self):
        test_list = []
        if self.queue_data:
            for job, values in self.queue_data.iteritems():
                if 'test_dir' in values.keys() and 'input_name' in values.keys():
                    test_dir = os.path.dirname(values['test_dir'])
                    input_name = values['input_name']
                    test_name = values['test_name']
                    test_list.append((test_dir, input_name, test_name))
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
            try:
                output = runCommand(command)
            except KeyboardInterrupt:
                self.cleanup(1)
            if 'ERROR' in output:
                print output
                self.cleanup(1)

    # Return path-friendly test name (remove special characters)
    def getJobName(self, tester):
        # A little hackish. But there is an instance or two where we do not have a tester 'object' that we
        # can to deal with, just the str(test_name)
        if type(tester) == type(str()) or type(tester) == type(u''):
            tester_text = tester
        else:
            tester_text = tester.specs['test_name']

        return ''.join(txt for txt in tester_text if txt.isalnum() or txt in ['_', '-'])

    # Return queue working directory
    # test_dir/job_<queue-filename>/
    def getWorkingDir(self, tester):
        return os.path.join(tester.getTestDir(), 'job_' + self.queue_file.name)

    # Return location of queue launch script
    # test_dir/job_<queue-filename>/<queue-filename>-test_name.sh
    def getQueueScript(self, tester):
        return os.path.join(self.getWorkingDir(tester), self.queue_file.name + '-' + self.getJobName(tester) + '.sh')

    # Alter 'skipped dependency' statuses where necessary
    def adjustSkippedStatus(self, tester):
        # A special case where this test is considered skipped due to prereqs not having a finished
        # status. Due to the way queueing works, we need to check on these prereq'd un-finished tests
        # and see if they are truly skipped/failed, or still pending in the queue. If they are pending,
        # we need to alter this test's status to reflect a 'HOLDING' pending status instead.

        # TODO: I do not like using logic based on the status _message_. But I see no other way to
        # determine the _why_ a test was skipped. To the TestHarness a skipped test is still a skipped
        # test. Adding additional buckets probably wouldn't help either. That would only create this mess
        # somewhere else... Better way?
        if tester.getStatusMessage() == 'skipped dependency':
            # get reverse dependencies
            reverse_deps = self.getDependents(tester)

            # Iterate through my dependents, and figure out if any of them are skipped or failed
            for test_name, dependents in reverse_deps.iteritems():
                for dependent in dependents:
                    tmp_job = self.getData(self.getJobName(dependent), skipped=True, did_fail=True)
                    if tmp_job['skipped'] or tmp_job['did_fail']:
                        # A dependent has been skipped or failed, so continue to let this job be flagged as
                        # a 'skipped dependency'
                        return

            # No dependents were found to be skipped or failed, so alter the status of this test from
            # skipped (due to a skipped dependency), to pending instead.
            tester.setStatus('HOLDING', tester.bucket_pending)

    # The Scheduler has updated a tester's status, so update that status in our queue_data
    def notifySchedulers(self, tester):
        if self.options.checkStatus:
            self.adjustSkippedStatus(tester)
        self.updateTesterStatus(tester)

    # canLaunch is the QueueManager's way of providing checkRunnable due to the multitude of modes the
    # TestHarness can operate in.
    def canLaunch(self, tester, checks, test_list):
        # We are processing a previous run
        if self.options.checkStatus:
            # Get this jobs previous status from queue_data
            job = self.getData(self.getJobName(tester),
                               depends_on_me=True,
                               status_message=True,
                               status_bucket=True)

            # Create a status_bucket according to what is stored in the json file and assign that status to
            # this tester
            bucket = self.createStatusBucket(job['status_bucket'])
            tester.setStatus(job['status_message'], bucket)

            # Reverse the dependency arrows
            # NOTE: I think this is right. Seems to work. Looks too simple to me
            if job['depends_on_me']:
                tester.specs['prereq'] = job['depends_on_me']

            # If this test has no dependents, just go.
            else:
                tester.specs['prereq'] = []

        # ask checkRunnable if we can run this test
        return tester.checkRunnableBase(self.options, checks, test_list)

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

        # We need to modify tester's test_dir to align with newly created directory the Queueing system is about
        # to create. This is so when we run tester.processResultsCommand, the correct paths are returned
        temp_test_dir = tester.getTestDir()
        tester.specs['test_dir'] = self.getWorkingDir(tester)

        # The command the tester needs to run for postProcessing (with the modified test_dir above)
        template_queue['post_command'] = ';'.join(tester.processResultsCommand(tester.getMooseDir(), self.options))

        # And now we set test_dir back again...
        tester.specs['test_dir'] = temp_test_dir


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
    def prepareQueueScriptBase(self, template_queue, tester):
        # Get a list of prereq tests this test may have
        prereq_ids = set()
        if tester.getPrereqs() != []:
            for prereq_test in tester.getPrereqs():
                job = self.getData(self.getJobName(prereq_test), id=True)
                prereq_ids.update([job['id']])

                # update queue_data with prereq test information
                self.updateData(self.getJobName(tester), prereq_tests=[prereq_test])

            # Update dependencies due to a prereq test
            self.updateDependencies(tester)

        # Augment derived queue script requirements
        template_queue = self.prepareQueueScript(template_queue, tester, prereq_ids)

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


        # Move into the newly created test directory
        current_pwd = os.getcwd()
        os.chdir(self.getWorkingDir(tester))

        # Copy files (unless they are listed in "no_copy")
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

        # Check to see if this test has any unsatisfied tests
        if self.unsatisfiedPrereqs(tester):
            self.queue.append([tester, tester_command, os.getcwd()])
            return

        # If we are trying to process results instead, skip these methods
        if not self.options.checkStatus:
            # Create and copy all the files/directories this test requires
            self.copyFiles(template_queue, tester)

            # Update the queue file
            self.updateTesterStatus(tester)

            # Call the derived method to build the queue batch script
            self.prepareQueueScriptBase(template_queue, tester)

        # This test already has a status we can return with
        else:
            if not tester.isPending():
                self.launched_jobs.add(tester.getTestName())
                self.handlePreviousStatus(tester)
                return

        # Move into the working_dir
        current_pwd = os.getcwd()
        os.chdir(self.getWorkingDir(tester))

        # We're good up to this point? Then launch the job!
        # NOTE: We do not want a slot check because we are submitting job to a third party queue.
        self.launchJob(tester, command, slot_check=False)

        # Return to the previous directory
        os.chdir(current_pwd)

    # A method to alter test_dir to reflect sub directory creation, and allow the tester to perform its
    # processResults method
    def processResults(self, tester):
        job = self.getData(self.getJobName(tester), std_out=True, test_dir=True, exit_code=True)
        with open(job['std_out'], 'r') as output_file:
            outfile = output_file.read()

        # Alter test_dir to reflect sub-directory creation
        original_testdir = tester.getTestDir()
        tester.specs['test_dir'] = job['test_dir']

        if tester.hasRedirectedOutput(self.options):
            outfile += self.getOutputFromFiles(tester)

        # Allow the tester to verify its own output and set the status
        output = tester.processResults(tester.specs['moose_dir'], job['exit_code'], self.options, outfile)

        # reset the original test_dir
        tester.specs['test_dir'] = original_testdir

        return output

    ## Handle a test which has previously ran
    def handlePreviousStatus(self, tester):
        finished_job = self.getData(self.getJobName(tester), std_out=True, test_dir=True, exit_code=True, did_pass=True)
        output = ''

        # If we want to read the perflog open the std_out file and read in its contents as output
        if self.options.timing and finished_job['did_pass']:
            with open(finished_job['std_out'], 'r') as f:
                output = self.readOutput(f)

        # The job previously finished with a failure (re-run the processResults method)
        if not finished_job['did_pass']:
            output = self.processResults(tester)

            # I suppose the test _might_ succeed this time
            if tester.didFail():
                self.skipped_jobs.add(tester.getTestName())
            else:
                self.finished_jobs.add(tester.getTestName())

        # The job finished
        else:
            self.finished_jobs.add(tester.getTestName())

        # Update the queue_data with possibly updated statuses/reasons
        self.updateTesterStatus(tester)

        # return to the TestHarness and print the results
        self.harness.testOutputAndFinish(tester, finished_job['exit_code'], output)

    ## Return control to the test harness by finalizing the test output and calling the callback
    def returnToTestHarness(self, job_index, output):
        (p, command, tester, time, f, slots) = self.jobs[job_index]

        # Get stdout from executed queue command
        queue_output = self.readOutput(f)

        # determine tester statuses depending on mode (launching or checking status)
        if self.options.checkStatus:
            # the test was already launched
            output += self.handleQueueStatus(tester, queue_output)
        else:
            # the test needed to be launched
            output += self.handleQueueLaunch(tester, queue_output)

        # Launching Jobs
        if not self.options.checkStatus and tester.isPending():
            self.finished_jobs.add(tester.getTestName())
            self.harness.handleTestResult(tester, output, tester.getStatusMessage(), add_to_table=False)

        # Launching Job or Checking Status and the test failed
        elif tester.didFail():
            self.skipped_jobs.add(tester.getTestName())
            self.harness.testOutputAndFinish(tester, p.returncode, output, time, clock())

        # Checking Status and the test is still pending
        elif self.options.checkStatus and tester.isPending():
            self.skipped_jobs.add(tester.getTestName())
            self.harness.testOutputAndFinish(tester, p.returncode, output, time, clock())

        # Checking Status and the test has finished
        elif self.options.checkStatus and not tester.isPending():
            self.finished_jobs.add(tester.getTestName())
            self.harness.testOutputAndFinish(tester, p.returncode, output, time, clock())

        # Update the queue_data with new test status reason
        self.updateTesterStatus(tester)
