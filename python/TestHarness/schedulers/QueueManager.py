from Scheduler import Scheduler
from collections import namedtuple
import os, sys, re, json, shutil, errno
from TestHarness import util

class QueueManager(Scheduler):
    """
    QueueManager is a Scheduler plugin responsible for allowing the testers to
    be scheduled via a third party queue system (like PBS) and to handle the
    stateful requirements associated with such a task (the session_file).
    """
    @staticmethod
    def validParams():
        params = Scheduler.validParams()
        params.addParam('no_copy_pattern', '.*\.sh',     "pattern of files not to copy")
        return params

    def __init__(self, harness, params):
        Scheduler.__init__(self, harness, params)

        # json storage
        self.__session_data = {}

        # Open existing session file
        if os.path.exists(self.options.session_file):
            self.__status_check = True
            try:
                self.__session_file = open(self.options.session_file, 'r+')
                self.__session_data = json.load(self.__session_file)

                # Set some important things that affect findAndRunTests (input file, --re)
                json_args = self.getData('QUEUEMANAGER',
                                         options_regexp=True,
                                         options_input=True,
                                         options_timing=True)
                self.options.input_file_name = json_args['options_input']

                # Honor any new reg_exp supplied by the user
                if self.options.reg_exp:
                    pass
                else:
                    self.options.reg_exp = json_args['options_regexp']

                # Only allow timing if user is asking, and user supplied those options
                # during initial launch phase (otherwise perflog will not be available).
                if not json_args['options_timing'] and self.options.timing:
                    self.options.timing = False


            except ValueError:
                print('Supplied session file: %s exists, but is not readable!' % (self.options.session_file))
                sys.exit(1)

        # session file does not exists. Create one instead.
        elif not self.options.queue_cleanup:
            self.__status_check = False
            self.__session_file = self.__createSessionFile()
            self.putData('QUEUEMANAGER',
                         options_regexp=self.options.reg_exp,
                         options_input=self.options.input_file_name,
                         options_timing=self.options.timing)

        self.params = params

    def __createSessionFile(self):
        """
        Private method to return a file object to store queue data in.
        """
        if self.options.session_file == 'generate':
            largest_serial_num = 0
            for name in os.listdir(self.harness.base_dir):
                m = re.search('queue_(\d{3})', name)
                if m != None and int(m.group(1)) > largest_serial_num:
                    largest_serial_num = int(m.group(1))
            self.options.session_file = "queue_" +  str(largest_serial_num+1).zfill(3)

        try:
            session_file = open(self.options.session_file, 'w')

        except IOError:
            print('Can not open %s for writing!' % (self.options.session_file))
            sys.exit(1)

        return session_file

    def __copyFiles(self, job, template):
        """ Copy the necessary test files the job will need in order to execute """
        tester = job.getTester()

        # Create regexp object of no_copy_pattern
        if 'no_copy_pattern' in self.params.keys():
            # Match no_copy_pattern value
            pattern = re.compile(self.params['no_copy_pattern'])
        else:
            # Match nothing if not set. Better way?
            pattern = re.compile(r'')

        # Create the job directory
        if not os.path.exists(self.getWorkingDir(job)):
            try:
                os.makedirs(self.getWorkingDir(job))
            except OSError, ex:
                if ex.errno == errno.EEXIST: pass
                else: raise

        source_dir = os.path.dirname(self.getWorkingDir(job))
        target_dir = self.getWorkingDir(job)

        # Copy files (unless they are listed in "no_copy")
        for file in os.listdir(source_dir):
            if os.path.isfile(os.path.join(source_dir, file)) and file != self.options.input_file_name and \
               (not tester.specs.isValid('no_copy') or file not in tester.specs['no_copy']) and \
               (not 'no_copy_pattern' in self.params.keys() or pattern.match(file) is None) and \
               not os.path.exists(os.path.join(target_dir, file)) and \
               os.path.splitext(file)[1] != '':
                shutil.copy(os.path.join(source_dir, file), os.path.join(target_dir, file))

        # Copy directories
        if 'copy_files' in template.keys():
            for file in template['copy_files']:
                if os.path.isfile(os.path.join(source_dir, file)):
                    if not os.path.exists(os.path.join(target_dir, file)):
                        shutil.copy(os.path.join(source_dir, file), os.path.join(target_dir, file))
                elif os.path.isdir(os.path.join(source_dir, file)):
                    try:
                        shutil.copytree(os.path.join(source_dir, file), os.path.join(target_dir, file))
                    except OSError, ex:
                        if ex.errno == errno.EEXIST: pass
                        else: raise Exception()

        # Create symlinks (do last, to allow hard copy above to be default)
        if tester.specs.isValid('link_files'):
            for file in tester.specs['link_files']:
                if os.path.exists(os.path.join(source_dir, file)):
                    try:
                        os.symlink(os.path.join(source_dir, file), target_dir)
                    except OSError, ex:
                        if ex.errno == errno.EEXIST: pass
                        else: raise Exception()

    def __createQueueScript(self, template):
        """ Create the launch script based on supplied template information """
        # Get a list of prereq tests this test may have
        f = open(self.params['queue_template'], 'r')
        content = f.read()
        f.close()

        queue_script = os.path.join(template['working_dir'], template['queue_script'])
        f = open(queue_script, 'w')

        # Do all of the replacements for the valid parameters
        for key in template.keys():
            if key.upper() in content:
                content = content.replace('<' + key.upper() + '>', str(template[key]))

        # Make sure we strip out any string substitution parameters that were not supplied
        for key in template.keys():
            if key.upper() not in content:
                content = content.replace('<' + key.upper() + '>', '')

        f.write(content)
        f.close()

    def __createStatusBucket(self, json_status):
        """
        private method to return a compatible tester status bucket
        """
        caveat = json_status['status_bucket']['status']
        caveat_color = json_status['status_bucket']['color']
        tmp_bucket = namedtuple('test_status', json_status['status_bucket'])

        return tmp_bucket(status=caveat, color=caveat_color)

    def _readJobOutput(self, job):
        """ boolean for reasons to open and read job output files """
        tester = job.getTester()
        return self.options.verbose \
            or self.options.timing \
            or tester.didFail()

    def checkStatusState(self):
        """ Return bool if we are processing status for launched jobs """
        return self.__status_check == True

    def handleJobStatus(self, job, output):
        """
        Call derived methods for handling third party command output.
        Return a dictionary of information which will be saved to our
        session storage for later use.
        """
        return {}

    def saveSession(self, job, **kwargs):
        """ Populate session storage with the current status of job """
        tester = job.getTester()

        self.putData(job.getUniqueIdentifier(),
                     caveat_message=tester.getStatusMessage(),
                     status_bucket=dict(tester.getStatus()._asdict()),
                     **kwargs)

    def getData(self, test_unique, **kwargs):
        """
        Return requested information as a dictionary if available.

        Syntax:
          dict = getData(str(unique_test_name), key=True)

        value = dict[key]
        """
        tmp_dict = {}
        if test_unique in self.__session_data.keys():
            for kkey in kwargs.keys():
                try:
                    tmp_dict[kkey] = self.__session_data[test_unique][kkey]
                except KeyError: # Requested key not available
                    tmp_dict[kkey] = None
        return tmp_dict

    def putData(self, test_unique, **kwargs):
        """
        Store requested information into the session.

        Syntax:
          putData(str(unique_test_name), **kwargs)

        """
        with self.dag_lock:
            for key, value in kwargs.iteritems():
                if test_unique in self.__session_data.keys():
                    self.__session_data[test_unique][key] = value
                else:
                    self.__session_data[test_unique] = {key : value}

    def writeSessionFile(self):
        """ Write the contents of your session to the session file """
        self.__session_file.seek(0)
        json.dump(self.__session_data, self.__session_file, indent=2)
        self.__session_file.truncate()
        self.__session_file.close()

    def cleanUp(self):
        """
        Remove all files/directories created during supplied session file
        """
        # Open existing session file
        if os.path.exists(self.options.session_file):
            try:
                session_file = open(self.options.queue_cleanup, 'r')
                session_data = json.load(session_file)
            except ValueError:
                print("Supplied session file %s is not readable!" % (self.options.queue_cleanup))
                sys.exit(1)
            # Iterate over session dictionary and perform delete operations
            for key in session_data.keys():
                if 'working_dir' in session_data[key]:
                    try:
                        shutil.rmtree(session_data[key]['working_dir'])
                    except OSError:
                        pass
            os.remove(self.options.queue_cleanup)

        else:
            print("%s does not exist!" % (self.options.queue_cleanup))
            sys.exit(1)

    def getWorkingDir(self, job):
        """ Return the queue working directory for job """
        tester = job.getTester()
        return os.path.join(tester.getTestDir(), 'job_' + self.options.session_file)

    def augmentQueueParamsBase(self, job):
        """ Build the queue execution script """
        template = {}
        tester = job.getTester()

        for param in self.params.keys():
            template[param] = self.params[param]

        # Set CPU request count
        # NOTE: we are calling tester.getProcs specifically, instead of through
        # the job class here, because we have altered each job to only require 1
        # process.
        template['mpi_procs'] = tester.getProcs(self.options)

        # Set a path friendly job name
        template['job_name'] = ''.join(txt for txt in tester.specs['test_name'] if txt.isalnum() or txt in ['_', '-'])

        # Set working directory
        template['working_dir'] = self.getWorkingDir(job)

        # Join the actual command we will execute inside the qsub script
        template['command'] = tester.getCommand(self.options)

        # #### create a set for copy and nocopy so its easier to work with
        no_copy_files = set([])
        copy_files = set([])

        if tester.specs.isValid('no_copy_files'):
            no_copy_files.update(tester.specs['no_copy_files'])
        if tester.specs.isValid('copy_files'):
            copy_files.update(tester.specs['copy_files'])
        if tester.specs.isValid('gold_dir'):
            copy_files.update([tester.specs['gold_dir']])

        # convert the copy and nocopy sets to flat lists
        template['copy_files'] = list(copy_files)
        template['no_copy'] = list(no_copy_files)

        # The location of the resulting execution script
        template['queue_script'] = template['job_name'] + '.sh'

        # Call derived augmentQueueParams to fill in specifics
        template = self.augmentQueueParams(job, template)

        return template

    def reserveSlots(self, job):
        """
        Inherited method which controls when jobs are allowed to execute,
        depending on available resources.

        QueueManager only executes third party queueing commands. So
        modify every job to only require 1 process.
        """
        job.setProcessors(1)
        return Scheduler.reserveSlots(self, job)

    def testOutput(self, job):
        """
        Adjust the Tester for a proper working directory (QueueManager creates sub-directores),
        and allow derived Tester to perform processResults.

        Return resulting output.
        """
        tester = job.getTester()
        json_job = self.getData(job.getUniqueIdentifier(), std_out=True, working_dir=True)

        if json_job['std_out']:
            stdout_file = os.path.join(json_job['working_dir'], json_job['std_out'])
        else:
            stdout_file=''

        if os.path.exists(stdout_file):
            file = open(stdout_file, 'r+')
            # do NOT use util.readOutput (trim output). Otherwise processResults may
            # fail to find something it needed.
            output = file.read()
        else:
            self.setJobStatus(job, tester.bucket_fail, 'NO STDOUT FILE')
            return ''

        # Alter test_dir to reflect working_dir creation
        tester.specs['test_dir'] = json_job['working_dir']

        if tester.hasRedirectedOutput(self.options):
            output += util.getOutputFromFiles(tester, self.options)

        # Allow the tester to verify its own output.
        output = tester.processResults(tester.specs['moose_dir'], self.options, output)

        # Set the testers output with modifications made above
        job.setOutput(output)

        # combine job stdout and processResults to the stdout file
        file.seek(0)
        file.write(output)
        file.truncate()
        file.close()

        return output

    def executeAndGetJobOutput(self, job):
        """
        Execute derived commands and obtain the output
        """
        json_data = self.getData(job.getUniqueIdentifier(), working_dir=True)
        output = util.runCommand(self.getQueueCommand(job), cwd=json_data['working_dir'])
        return output

    def getCurrentJobStatus(self, job):
        """
        Set a job to the most current known status.

        This method will first check if job has a previous pass/fail
        status. If so, it sets that status with out performing any
        third party queueing commands.

        If the job has a status of queued, we call derived methods to
        determine current status as the third party queueing system
        sees it.
        """
        tester = job.getTester()
        json_status = self.getData(job.getUniqueIdentifier(),
                                   status_bucket=True,
                                   caveat_message=True,
                                   std_out=True,
                                   working_dir=True)
        if json_status:
            bucket = self.__createStatusBucket(json_status)

        # This test was not included during the launch sequence
        else:
            self.setJobStatus(job, tester.bucket_silent, 'NO LAUNCH INFORMATION')
            return

        self.setJobStatus(job, bucket, json_status['caveat_message'])

        job_information = {}

        # Job was queued previously, and we want to know if the status has changed.
        if tester.isQueued():
            output = self.executeAndGetJobOutput(job)
            job_information = self.handleJobStatus(job, output)

        # Some other finished status. Check conditionals if user wants to re-open stdout.
        elif self._readJobOutput(job):
            json_job = self.getData(job.getUniqueIdentifier(), std_out=True, working_dir=True)
            stdout_file = os.path.join(json_job['working_dir'], json_job['std_out'])
            if os.path.exists(stdout_file):
                with open(stdout_file, 'r') as f:
                    # We can use trimmed output here, now that the job has a proper
                    # status (we are not going to run processResults again).
                    outfile = util.readOutput(f, self.options)
                job.setOutput(outfile)
            else:
                self.setJobStatus(job, tester.bucket_fail, 'NO STDOUT FILE')

        # Update session storage with possibly new job status information
        self.saveSession(job, **job_information)

    def checkJobStatus(self, job):
        """
        Check status for job. Depending on that status, this method
        does the following:

        Waiting;         Run processResults.
        Anything else;   Update session storage with any
                         new information.
        """
        tester = job.getTester()
        self.getCurrentJobStatus(job)

        # This job is possibly ready. But we need to make sure all downstream jobs are
        # ready as well.
        if tester.isWaiting():
            job_dag = job.getOriginalDAG()
            downstream_jobs = job_dag.all_downstreams(job)

            for downstream_job in downstream_jobs:
                d_tester = downstream_job.getTester()

                # Determine, set, and save our children's status
                self.getCurrentJobStatus(downstream_job)

                # A child job is still not ready. We need to adjust our status to match.
                if d_tester.isQueued():
                    self.setJobStatus(job, tester.bucket_queued, 'WAITING')
                    return

            self.testOutput(job)

        # Save resulting status to the session storage
        self.saveSession(job)

    def jobLaunch(self, job):
        """ Prepare for and launch a job """
        tester = job.getTester()

        # Do once (prepare job)
        if tester.isPending():
            # Augment queue parameters
            template = self.augmentQueueParamsBase(job)

            # Prepare the worker directory
            self.__copyFiles(job, template)

            # Create the shell execution script
            self.__createQueueScript(template)

            # Save template information
            self.putData(job.getUniqueIdentifier(),
                         queue_script=template['queue_script'],
                         working_dir=template['working_dir'],
                         job_name=template['job_name'])

        # Get derived launch command and launch this job (blocking)
        output = self.executeAndGetJobOutput(job)

        # Call derived method to set job status and obtain information
        # the third party scheduler wishes to save to the session storage
        job_information = self.handleJobStatus(job, output)
        self.saveSession(job, **job_information)

    def postRun(self, job):
        """
        Called after a job has finished running.

        This method will call derived postLaunchCommand to obtain
        a possible command to run, once this group of jobs has
        launched.
        """
        tester = job.getTester()
        launchable_jobs = []

        # Check if all jobs in this group are pending (meaning they were launched)
        if tester.isQueued() and not self.__status_check:
            current_dag = job.getDAG()

            # If current_dag is empty this is the last job to have been removed
            if current_dag.size() == 0:
                original_dag = job.getOriginalDAG()

                for ind_job in original_dag.topological_sort():
                    launchable_jobs.append(ind_job)

        # Ask derived scheduler to build a postRun command
        if launchable_jobs:
            command = self.postLaunchCommand(launchable_jobs)
            if command:
                util.runCommand(command)

    def preLaunch(self, job_dag):
        """ Reverse the test order when we are trying to process results """
        if self.__status_check:
            job_dag.reverse_edges()

    def notifyFinishedSchedulers(self):
        """
        Save session results to a file.
        """
        # Write session data to file
        self.writeSessionFile()

    def reportSkipped(self, jobs):
        """ Save skipped job information in our session storage """
        # We don't want to save silently skipped tests
        for job in jobs:
            tester = job.getTester()
            if tester.isSilent():
                return
            else:
                self.saveSession(job)

    def run(self, job):
        """
        Entry point to the QueueManager.

        Supply a job to have that job launched into a third party queueing
        system. Once that job launches, the results will be written to a
        json file (called a session file through out this class), and then
        we exit.

        Calling run again, supplying the same job while using the same
        session file, will instruct this class to process the job results
        if it is ready to do so. Those results will also be saved to the
        session file for later use (for faster results on consecutive
        calls).
        """
        if self.__status_check:
            self.checkJobStatus(job)
        else:
            self.jobLaunch(job)
