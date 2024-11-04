#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import itertools, re, os, time, threading, traceback
from timeit import default_timer as clock
from TestHarness.StatusSystem import StatusSystem
from TestHarness.FileChecker import FileChecker
from TestHarness.runners.Runner import Runner
from TestHarness import OutputInterface, util
from tempfile import TemporaryDirectory
from collections import namedtuple

from TestHarness import util

def time_now():
    return time.time_ns() / (10 ** 9)

class Timer(object):
    """
    A helper class for testers to track the time it takes to run.
    """
    def __init__(self):
        # Dict of time name -> (start,) or (start,end)
        self.times = {}
        # Threading lock for setting timers
        self.lock = threading.Lock()

    @staticmethod
    def time_now() -> float:
        """ Helper for getting a precise now time """
        return float(time.time_ns() / (10 ** 9))

    def start(self, name: str, at_time=None):
        """ Start the given timer """
        if not at_time:
            at_time = self.time_now()
        with self.lock:
            self.times[name] = [at_time]

    def stop(self, name: str, at_time=None):
        """ End the given timer """
        if not at_time:
            at_time = self.time_now()
        with self.lock:
            entry = self.times.get(name)
            if not entry:
                raise Exception(f'Missing time entry {name}')
            if len(entry) > 1:
                raise Exception(f'Time entry {name} already stopped')
            entry.append(at_time)

    def startMain(self):
        """ Get the start time for the main timer """
        self.start('main')

    def stopMain(self):
        """ Get the end time for the main timer """
        self.stop('main')

    def hasTime(self, name: str):
        """ Whether or not the given timer exists """
        with self.lock:
            return name in self.times

    def hasTotalTime(self, name: str):
        """ Whether or not the given total time exists """
        with self.lock:
            entry = self.times.get(name)
            if not entry:
                return False
            return len(entry) > 1

    def totalTime(self, name='main'):
        """ Get the total time for the given timer """
        with self.lock:
            entry = self.times.get(name)
            if not entry:
                if name == 'main':
                    return 0
                raise Exception(f'Missing time entry {name}')

            if len(entry) > 1:
                return entry[1] - entry[0]
            return time_now() - entry[0]

    def totalTimes(self):
        """ Get the total times """
        times = {}
        for name, entry in self.times.items():
            times[name] = self.totalTime(name)
        return times

    def startTime(self, name):
        """ Get the start time """
        with self.lock:
            entry = self.times.get(name)
            if not entry:
                raise Exception(f'Missing time entry {name}')
            return entry[0]

    def reset(self, name = None):
        """ Resets a given timer or all timers """
        with self.lock:
            if name:
                if name not in self.times:
                    raise Exception(f'Missing time entry {name}')
                del self.times[name]
            else:
                self.times.clear()

    class TimeManager:
        """ Context manager for timing a section """
        def __init__(self, timer, name: str):
            self.timer = timer
            self.name = name
        def __enter__(self):
            self.timer.start(self.name)
        def __exit__(self, exc_type, exc_val, exc_tb):
            self.timer.stop(self.name)

    def time(self, name: str):
        """ Time a section using a context manager """
        return self.TimeManager(self, name)

class Job(OutputInterface):
    """
    The Job class is a simple container for the tester and its associated output file object, the DAG,
    the process object, the exit codes, and the start and end times.
    """
    # Iterator for producing a unique Job ID
    id_iter = itertools.count()

    # Thread lock for creating output directories
    mkdir_lock = threading.Lock()

    # Tuple for getJointStatus()
    JointStatus = namedtuple('JointStatus', ['status', 'message', 'color', 'status_code', 'sort_value'])

    def __init__(self, tester, job_dag, options):
        OutputInterface.__init__(self)

        self.id = next(self.id_iter)
        self.options = options
        self.__j_lock = threading.Lock()
        self.__tester = tester
        self.specs = tester.specs
        self.__job_dag = job_dag
        self.timer = Timer()
        self.__joined_out = ''
        self.report_timer = None
        self.__slots = None

        # Create a fileChecker object to be able to call filecheck methods
        self.fileChecker = FileChecker(self.options.input_file_name)

        # List of files modified by this job.
        self.modifiedFiles = []

        # Set of all jobs that this job races with.
        self.racePartners = set()

        # Alternate text we want to print as part of our status instead of the
        # pre-formatted status text (LAUNCHED (pbs) instead of FINISHED for example)
        self.__job_message = ''

        ### Enumerate the job statuses we want to use
        self.job_status = StatusSystem(locking=True)

        self.hold = self.job_status.hold
        self.queued = self.job_status.queued
        self.running = self.job_status.running
        self.skip = self.job_status.skip
        self.error = self.job_status.error
        self.timeout = self.job_status.timeout
        self.finished = self.job_status.finished

        self.__finished_statuses = [self.skip,
                                    self.error,
                                    self.timeout,
                                    self.finished]

        self.__pending_statuses = [self.hold,
                                   self.queued,
                                   self.running]

        # Initialize jobs with a holding status
        self.setStatus(self.hold)

        # Whether or not we should forcefully report the status of this Job
        # the next time report statuses
        self.force_report_status = False

        # The object that'll actually do the run
        self._runner = None

        # A temp directory for this Job, if requested
        self.tmp_dir = None

    def __del__(self):
        # Do any cleaning that we can (removes the temp dir for now if it exists)
        self.cleanup()

    def getID(self):
        """Returns the unique ID for the job"""
        return self.id

    def setRunner(self, runner: Runner):
        """Sets the underlying Runner object that will run the command"""
        self._runner = runner

    def getLock(self):
        """ Get the lock associated with this job """
        return self.__j_lock

    def getTempDirectory(self):
        """
        Gets a shared temp directory that will be cleaned up for this Tester
        """
        if self.tmp_dir is None:
            self.tmp_dir = TemporaryDirectory(prefix='tester_')
        return self.tmp_dir

    def cleanup(self):
        """
        Entry point for doing any cleaning if necessary.

        Currently just cleans up the temp directory
        """
        if self.tmp_dir is not None:
            # Don't let this fail
            try:
                self.tmp_dir.cleanup()
            except:
                pass
            self.tmp_dir = None

    def getUpstreams(self):
        """ Return a list of all the jobs that needed to be completed before this job """
        dag = self.getDAG()
        original_dag = dag.getOriginalDAG()
        return dag.predecessors(self, original_dag)

    def getDownstreams(self):
        """ Return a list of all the jobs that need this job completed """
        dag = self.getDAG()
        original_dag = dag.getOriginalDAG()
        return dag.all_downstreams(self, original_dag)

    def getDAG(self):
        """ Return the DAG associated with this tester """
        return self.__job_dag.getDAG()

    def getTester(self):
        """ Return the tester object """
        return self.__tester

    def getSpecs(self):
        """ Return tester spec params """
        return self.getTester().specs

    def getTestName(self):
        """ Wrapper method to return the testers test name """
        return self.__tester.getTestName()

    def getTestNameShort(self):
        """ Return the shorthand Test name """
        return self.__tester.getTestNameShort()

    def getPrereqs(self):
        """ Wrapper method to return the testers prereqs """
        return self.__tester.getPrereqs()

    def getTestDir(self):
        """ Wrapper method to return the testers working directory """
        return self.__tester.getTestDir()

    def addCaveats(self, kwargs):
        """ Wrapper method for setting caveats """
        return self.__tester.addCaveats(kwargs)

    def removeCaveat(self, caveat):
        """ Wrapper method for removing caveats """
        return self.__tester.removeCaveat(caveat)

    def getCaveats(self):
        """ Wrapper method for getting caveats """
        return self.__tester.getCaveats()

    def clearCaveats(self):
        """ Wrapper method for clearing caveats """
        return self.__tester.clearCaveats()

    def getCommand(self):
        """ Wrapper method for returing command """
        return self.__tester.getCommand(self.options)

    def getCommandRan(self):
        """ Wrapper method for returing command ran """
        return self.__tester.getCommandRan()

    def getRunnable(self):
        """ Wrapper method to return getRunnable """
        return self.__tester.getRunnable(self.options)

    def getOutputFiles(self, options):
        """ Wrapper method to return getOutputFiles (absolute path) """
        files = []
        for file in self.__tester.getOutputFiles(options):
            files.append(os.path.join(self.__tester.getTestDir(), file))
        return files

    def getMaxTime(self):
        """ Wrapper method to return getMaxTime """
        return self.__tester.getMaxTime()

    def getInputFile(self):
        """ Wrapper method to return input filename """
        return self.__tester.getInputFile()

    def getInputFileContents(self):
        """ Wrapper method to return input file contents """
        return self.__tester.getInputFileContents()

    def getUniqueIdentifier(self):
        """ A unique identifier for this job object """
        return os.path.join(self.getTestDir(), self.getTestName())

    def getUniquePrereqs(self):
        """ Return a list of prereqs with what should be their unique identifier """
        prereqs = self.getPrereqs()
        unique_prereqs = []
        for prereq in prereqs:
            unique_prereqs.append(os.path.join(self.getTestDir(), prereq))
        return unique_prereqs

    def getSlots(self):
        """ Return the number of slots this job consumes """
        if self.__slots == None:
            return self.setSlots(self.__tester.getSlots(self.options))
        return self.__slots

    def setSlots(self, slots):
        """ Set the number of processors this job consumes """
        self.__slots = int(slots)
        return self.__slots

    def canParallel(self):
        """ Call DAG and determine if this group can run in parallel """
        return self.__job_dag.canParallel()

    def run(self):
        """
        A blocking method to handle the exit status of the process object while keeping track of the
        time the process was active. When the process exits, read the output and close the file.
        """
        tester = self.__tester

        # Start the main timer for running
        self.timer.startMain()

        # Helper for exiting
        def finalize():
            # Run cleanup
            with self.timer.time('job_cleanup'):
                self.cleanup()
            # Sanitize the output from all objects
            self.sanitizeAllOutput()
            # Stop timing
            self.timer.stopMain()

        # Set the output path if its separate and initialize the output
        if self.hasSeperateOutput():
            # Need to potentially create the output directory
            self.createOutputDirectory()

            # Failed to create the directory
            if self.isError():
                finalize()
                return

            # Set the output path for each object
            for name, object in self.getOutputObjects().items():
                output_path = self.getOutputPathPrefix() + f'.{name}_out.txt'
                object.setSeparateOutputPath(output_path)
                object.clearOutput()

        # Helper for trying and catching
        def try_catch(do, exception_name, timer_name):
            with self.timer.time(timer_name):
                failed = False
                try:
                    do()
                except:
                    trace = traceback.format_exc()
                    self.setStatus(self.error, f'{exception_name} EXCEPTION')
                    self.appendOutput(util.outputHeader('Python exception encountered') + trace)
                    failed = True

            if failed:
                finalize()
            return not failed

        # Do not execute app, but still run the tester
        # This is truly awful and I really hate that it got put in here,
        # please remove it if you can.
        if not tester.shouldExecute():
            run_tester = lambda: tester.run(self.options, 0, '')
            try_catch(run_tester, 'TESTER RUN', 'tester_run')
            return

        if self.options.pedantic_checks and self.canParallel():
            # Before the job does anything, get the times files below it were last modified
            with self.timer.time('pedantic_init'):
                self.fileChecker.get_all_files(self, self.fileChecker.getOriginalTimes())
                self.addCaveats('pedantic check')
                time.sleep(1)

        with self.timer.time('tester_prepare'):
            tester.prepare(self.options)

        # Verify that the working directory is available right before we execute
        if not os.path.exists(tester.getTestDir()):
            self.setStatus(self.error, 'WORKING DIRECTORY NOT FOUND')
            finalize()
            return
        # Getting the command can also cause a failure, so try that
        tester.getCommand(self.options)
        if tester.isError():
            finalize()
            return

        # Spawn the process
        spawn = lambda: self._runner.spawn(self.timer)
        if not try_catch(spawn, 'RUNNER SPAWN', 'runner_spawn'):
            return

        # Entry point for testers to do other things
        post_spawn = lambda: tester.postSpawn(self._runner)
        if not try_catch(post_spawn, 'TESTER POST SPAWN', 'tester_post_spawn'):
            return

        # And wait for it to complete
        wait = lambda: self._runner.wait(self.timer)
        if not try_catch(wait, 'RUNNER WAIT', 'runner_wait'):
            return

        # Job error occurred, which means the Runner didn't complete
        # so don't process anything else
        if self.isError():
            finalize()
            return

        # And do finalize (really just cleans up output)
        runner_finalize = lambda: self._runner.finalize()
        if not try_catch(runner_finalize, 'RUNNER FINALIZE', 'runner_finalize'):
            finalize()
            return

        # Exit if we have bad output in the runner before running the tester
        self.sanitizeAllOutput()
        if self.isError():
            finalize()
            return

        # Check if the files we checked on earlier were modified.
        if self.options.pedantic_checks and self.canParallel():
            with self.timer.time('pedantic_check'):
                self.fileChecker.get_all_files(self, self.fileChecker.getNewTimes())
                self.modifiedFiles = self.fileChecker.check_changes(self.fileChecker.getOriginalTimes(),
                                                                    self.fileChecker.getNewTimes())

        # Allow derived proccessResults to process the output and set a failing status (if it failed)
        runner_output = self._runner.getRunOutput().getOutput()
        exit_code = self._runner.getExitCode()
        run_tester = lambda: tester.run(self.options, exit_code, runner_output)
        try_catch(run_tester, 'TESTER RUN', 'tester_run')

        # Run finalize now that we're done
        finalize()

    def killProcess(self):
        """ Kill remaining process that may be running """
        if self._runner:
            try:
                self._runner.kill()
            except:
                pass
        self.cleanup()

    def getOutputObjects(self) -> dict:
        """
        Get a dict of all of the objects that contribute to output

        The key is a name which is a human readable name of the object
        """
        objects = {}
        if self.getRunner():
            objects['runner_run'] = self.getRunner().getRunOutput()
            objects['runner'] = self.getRunner()
        objects['tester'] = self.getTester()
        objects['job'] = self
        return objects

    def getCombinedSeparateOutputPaths(self):
        """
        Gets a dict of all of the --sep-files file paths that were produced

        The key is a name which is a human readable name of the object
        """
        paths = {}
        for name, object in self.getOutputObjects().items():
            paths[name] = object.getSeparateOutputFilePath() if object.hasOutput() else None
        return paths

    def getAllOutput(self) -> dict:
        """ Get all output in a dict from each object to the text output """
        output = {}
        for name, object in self.getOutputObjects().items():
            output[name] = object.getOutput()
        return output

    def sanitizeAllOutput(self):
        """ Sanitizes the output from all output objects

        If output is retreived from these objects via getOutput() and
        it contains bad output, it will throw an error. Instead of
        throwing an error, we will sanitize it before hand and then
        set a Job error so that we can still continue in a failed state.
        """
        all_failures = []
        for name, object in self.getOutputObjects().items():
            failures = object.sanitizeOutput()
            all_failures.extend([s + f' in {name}' for s in failures])
        if all_failures:
            self.setStatus(self.error, ', '.join(all_failures))

    def getOutputForScreen(self):
        """ Gets the output for printing on screen """
        show_output = self.options.verbose or (self.isFail() and not self.options.quiet) or self.isError()
        if not show_output:
            return None

        if self.getCommandRan():
            command = self.getCommandRan()
        else:
            command = self.getCommand()

        output = 'Working Directory: ' + self.getTestDir() + '\nRunning command: ' + command + '\n'

        # Whether or not to limit the runner_run output, which is the output from the
        # actual run (the process that the harness runs)
        limit_runner_run = None
        if self.options.sep_files and not self.options.verbose:
            limit_runner_run = '-x/--sep-files'

        options = self.options
        specs = self.specs

        for name, object in self.getOutputObjects().items():
            object_output = object.getOutput()

            # Nothing to output
            if not object_output:
                continue

            # Max size of this output for trimming
            # Currently only used for the runner_run output
            max_size = None

            # Possibly trim or skip the runner_run output (actual process output)
            if name == 'runner_run':
                # Limit the runner run output
                if limit_runner_run:
                    output_split = object_output.splitlines()
                    max_lines = 100
                    if len(output_split) > max_lines:
                        prefix = f'Displaying last {max_lines} lines due to {limit_runner_run}\n\n'
                        object_output = prefix + '\n'.join(output_split[-max_lines:])

                # Default trimmed output size
                max_size = 100000
                # max_buffer_size is set
                if specs.isValid('max_buffer_size'):
                    # ...to the max
                    if specs['max_buffer_size'] == -1:
                        max_size = None
                    # ... or to a value
                    else:
                        max_size = int(specs['max_buffer_size'])
                # Disable trimmed output
                if options.no_trimmed_output:
                    max_size = None
                # Don't trim output on error, and we errored
                if options.no_trimmed_output_on_error and self.isFail():
                    max_size = None

            # Add a complete line break between objects
            if output:
                output += '\n'
            # Add a header before the output starts
            output += util.outputHeader(f'Begin {name} output', ending=False) + '\n'
            # Remove extra line breaks
            object_output = object_output.lstrip().rstrip()
            # Add the output, trimming if needed
            output += util.trimOutput(object_output.lstrip().rstrip(), max_size=max_size)
            # Add a newline as we ran rstrip
            output += '\n'
            # Add a footer after the output ends
            output += '\n' + util.outputHeader(f'End {name} output', ending=False)

        # Add the text name prefix
        if output:
            lines = output.split('\n')
            joint_status = self.getJointStatus()
            prefix = util.colorText(self.getTestName() + ': ', joint_status.color,
                                    colored=self.options.colored, code=self.options.code)
            output = prefix + ('\n' + prefix).join(lines)

        return output

    def getRunner(self):
        """ Gets the Runner that actually runs the command """
        return self._runner

    def getOutputDirectory(self):
        """ Get the directory for output for this job """
        if not self.options.output_dir:
            return self.getTestDir()
        return os.path.join(self.options.output_dir, self.getTestName()[:-len(self.getTestNameShort())-1])

    def createOutputDirectory(self):
        """ Create the output directory for this job, if needed """
        if not self.options.output_dir:
            return
        output_dir = self.getOutputDirectory()
        with Job.mkdir_lock:
            if not os.path.isdir(output_dir):
                try:
                    os.makedirs(output_dir)
                except OSError as ex:
                    if ex.errno == errno.EEXIST:
                        pass
                    else:
                        self.setStatus(self.error, f'DIRECTORY CREATION FAILURE')
                        self.appendOutput(f'Failed to create Job directory {output_dir}')

    def getOutputPathPrefix(self):
        """
        Returns a file prefix that is unique to this job

        Should be used for all TestHarness produced files for this job
        """
        return os.path.join(self.getOutputDirectory(), self.getTestNameShort().replace(os.sep, '.'))

    def hasSeperateOutput(self):
        """
        Whether or not this job has separate output.

        That is, whether or not we should pipe output to a file
        """
        return self.options.sep_files

    def getTiming(self):
        """ Return active time if available, if not return a comparison of start and end time """
        # Actual execution time
        if self.timer.hasTime('runner_run'):
            return self.timer.totalTime('runner_run')
        # Job has started
        if self.timer.hasTime('main'):
            return self.timer.totalTime()
        return 0.0

    def getStatus(self):
        return self.job_status.getStatus()

    def setStatus(self, status, message=''):
        if self.isFinished():
            return self.getStatus()
        # Try and preserve job messages
        if not self.__job_message and message:
            self.__job_message = message
        return self.job_status.setStatus(status)

    def createStatus(self):
        return self.job_status.createStatus()

    def previousTesterStatus(self):
        return self.__tester.previousTesterStatus(self.options)

    def getStatusMessage(self):
        return self.__job_message

    ### Boolean status comparisons based on current Job _and_ Tester status. All finalized status calls
    ### should now call job.isSomeStatus for the definitive answer.
    # the following are more job related...
    def isError(self):
        return self.getStatus() in self.job_status.getFailingStatuses()

    def isSkip(self):
        _status = self.getStatus()
        return (_status == self.finished and self.__tester.isSkip()) \
            or (_status == self.skip and self.isNoStatus()) \
            or (_status == self.skip and self.__tester.isSkip())

    def isHold(self):
        return self.getStatus() == self.hold
    def isQueued(self):
        _status = self.getStatus()
        return (_status == self.queued and self.isNoStatus()) \
            or (_status in self.__finished_statuses and self.__tester.isQueued())
    def isRunning(self):
        return self.getStatus() in self.job_status.getPendingStatuses()
    def isTimeout(self):
        return self.getStatus() == self.timeout
    def isPending(self):
        return self.getStatus() in self.__pending_statuses
    def isFinished(self):
        return self.getStatus() in self.__finished_statuses

    # the following more tester related...
    def isSilent(self):
        return self.__tester.isSilent()
    def isNoStatus(self):
        return self.__tester.isNoStatus()
    def isSilent(self):
        return self.__tester.isSilent() or (not self.options.report_skipped and self.isSkip())
    def isPass(self):
        return self.__tester.isPass()
    def isFail(self):
        return self.__tester.isFail() or self.isError()
    def isDiff(self):
        return self.__tester.isDiff()
    def isDeleted(self):
        return self.__tester.isDeleted()

    def getJointStatus(self):
        """
        Return the most accurate message possible, based on the statuses of both the Tester and Job.
        Most of the time, we want to return a Tester status. As this class will have the most detail.
        """
        # Job has failed, or tester has no status
        if self.isError() or self.isNoStatus():
            return Job.JointStatus(status=self.getStatus().status,
                                   message=self.getStatusMessage(),
                                   color=self.getStatus().color,
                                   status_code=self.getStatus().code,
                                   sort_value=self.getStatus().sort_value)

        # Tester has a finished status of some sort
        return Job.JointStatus(status=self.__tester.getStatus().status,
                               message=self.__tester.getStatusMessage(),
                               color=self.__tester.getStatus().color,
                               status_code=self.__tester.getStatus().code,
                               sort_value=self.__tester.getStatus().sort_value)

    def storeResults(self, scheduler):
        """ Store the results for this Job into the results storage """
        joint_status = self.getJointStatus()

        # Base job data
        job_data = {'timing'               : self.timer.totalTimes(),
                    'status'               : joint_status.status,
                    'status_message'       : joint_status.message,
                    'fail'                 : self.isFail(),
                    'color'                : joint_status.color,
                    'caveats'              : list(self.getCaveats()),
                    'tester'               : self.getTester().getResults(self.options)}
        if self.hasSeperateOutput():
            job_data['output_files'] = self.getCombinedSeparateOutputPaths()
        else:
            job_data['output'] = self.getAllOutput()

        # Extend with data from the scheduler, if any
        job_data.update(scheduler.appendResultFileJob(self))

        # Get the entry we're loading into
        test_dir_entry, test_entry = self.getTester().getResultsEntry(self.options, True)

        # Add the job data
        test_entry.update(job_data)

    def loadPreviousResults(self):
        """ Loads the previous results for this job for the results storage """
        # False here means don't create it
        test_dir_entry, test_entry = self.getTester().getResultsEntry(self.options, False)

        # Set the tester status
        tester = self.getTester()
        status, message, caveats = self.previousTesterStatus()
        tester.setStatus(status, message)
        if caveats:
            tester.addCaveats(caveats)

        # Set the previous times
        self.timer.reset()
        time_now = Timer.time_now()
        for name, total_time in test_entry['timing'].items():
            self.timer.start(name, time_now)
            self.timer.stop(name, time_now + total_time)

        # Load the output
        output_files = test_entry.get('output_files')
        output = test_entry.get('output')
        for name, object in self.getOutputObjects().items():
            if output_files: # --sep-files
                object.setSeparateOutputPath(output_files[name])
            elif output: # stored in result
                object.setOutput(output[name])
            else:
                raise Exception(f'Test {self.getTestName()} missing output')
