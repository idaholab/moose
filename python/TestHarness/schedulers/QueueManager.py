from Scheduler import Scheduler
from collections import namedtuple
import os, sys, re, json, shutil, errno
from TestHarness import util

class QueueManagerError(Exception):
    pass

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

        # a set containing any launched jobs
        self.__jobs = set([])

        # Open existing session file
        if os.path.exists(self.options.session_file):
            self.__status_check = True
            try:
                self.__session_file = open(self.options.session_file, 'r+')
                self.__session_data = json.load(self.__session_file)

                # Set some important things that affect findAndRunTests (input file, --re)
                json_args = self.getData('QUEUEMANAGER',
                                         options_regexp=True,
                                         options_input=True)
                self.options.reg_exp = json_args['options_regexp']
                self.options.input_file_name = json_args['options_input']

            except ValueError:
                raise QueueManagerError('Supplied session file: %s exists, but is not readable!' % (self.options.session_file))

        # session file does not exists. Create one instead.
        else:
            self.__status_check = False
            self.__session_file = self.__createSessionFile()
            self.putData('QUEUEMANAGER',
                         options_regexp=self.options.reg_exp,
                         options_input=self.options.input_file_name)

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
            raise QueueManagerError('Can not open %s for writing!' % (self.options.session_file))

        return session_file

    def __copyFiles(self, job_container, template):
        """ Copy the necessary test files the job will need in order to execute """
        tester = job_container.getTester()

        # Create regexp object of no_copy_pattern
        if 'no_copy_pattern' in self.params.keys():
            # Match no_copy_pattern value
            pattern = re.compile(self.params['no_copy_pattern'])
        else:
            # Match nothing if not set. Better way?
            pattern = re.compile(r'')

        # Create the job directory
        if not os.path.exists(self.getWorkingDir(job_container)):
            try:
                os.makedirs(self.getWorkingDir(job_container))
            except OSError, ex:
                if ex.errno == errno.EEXIST: pass
                else: raise

        # Move into the newly created test directory
        current_pwd = os.getcwd()
        os.chdir(self.getWorkingDir(job_container))

        # Copy files (unless they are listed in "no_copy")
        for file in os.listdir('../'):
            if os.path.isfile('../' + file) and file != self.options.input_file_name and \
               (not tester.specs.isValid('no_copy') or file not in tester.specs['no_copy']) and \
               (not 'no_copy_pattern' in self.params.keys() or pattern.match(file) is None) and \
               not os.path.exists(file) and \
               os.path.splitext(file)[1] != '':
                shutil.copy('../' + file, '.')

        # Copy directories
        if 'copy_files' in template.keys():
            for file in template['copy_files']:
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

    def __prepareQueueScript(self, template):
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

    def __getStatusBucket(self, job_container):
        """ Method used to create a tester status bucket """
        test_unique = self.getUnique(job_container)
        data = self.getData(test_unique, status_bucket=True)
        caveat = data['status_bucket']['status']
        caveat_color = data['status_bucket']['color']
        tmp_bucket = namedtuple('test_status', data['status_bucket'])

        return tmp_bucket(status=caveat, color=caveat_color)

    def __useImmediateStatus(self, job_container):
        """
        Determin if the previous status in our current state, or current status
        in current state requires a processResults operation. Say that three
        times fast.
        """
        tester = job_container.getTester()

        # Attempt to adjust tester status if we feel this test has previously run
        if self.__status_check and (tester.isPending() or tester.isInitialized()):
            test_unique = self.getUnique(job_container)
            status_bucket = self.__getStatusBucket(job_container)
            caveat = self.getData(test_unique, caveat_message=True)

            # Set the tester to what ever the status was set in the session file, overriding the
            # tester's current status of Pending.
            tester.setStatus(caveat['caveat_message'], status_bucket)

        # Statuses that do not require another processResults operation
        if tester.didPass() \
           or tester.isSkipped() \
           or tester.isDeleted() \
           or tester.isSilent():
            return True

    def processJobs(self, jobs):
        """
        Perform status checks on 'jobs' already launched.

        This method is special in the fact that it will iterate over
        a reversed list of concurrent jobs generated by the DAG object
        contained in 'jobs' rather than the supplied ordered list of
        'jobs'. This also means we need to handle past jobs that were
        skipped separately (skipped tests will not be contained in the
        DAG).
        """
        launched_dags = []
        for job_container in jobs:
            tester = job_container.getTester()
            test_unique = self.getUnique(job_container)

            # Print the skipped variety tests now, which will not be in the DAG.
            if tester.isSkipped() or tester.isSilent() or tester.isDeleted():
                self.statusJobs([job_container])
                continue

            # Only interested in jobs contained in the current session
            if test_unique not in self.__session_data.keys():
                continue

            # Get the DAGs we need
            job_dag = job_container.getDAG()
            original_dag = job_container.getOriginalDAG()
            reverse_dag = original_dag.reverse_edges()

            # job_dag is unique and shared amongst a group of job_containers. We only want
            # to work with one of these DAGs per group. So this is how we do it.
            if job_dag in launched_dags:
                continue
            launched_dags.append(job_dag)

            # Run the postProcess checks in reverse order
            for original_job_container in reverse_dag.topological_sort():
                # Use a previous statuses whenever possible
                if self.__useImmediateStatus(original_job_container):
                    self.statusJobs([original_job_container])
                    continue

                # Generate and run appropriate command for current state of operation
                queue_command = self.getQueueCommand(original_job_container)
                output = util.runCommand(queue_command)

                # Call derived methods to set a job status
                self.handleJobStatus(original_job_container, output)

                # Print the results
                self.statusJobs([original_job_container])

    def statusJobs(self, jobs):
        """
        Print the status for list of 'jobs'.

        Depending on the status of the test, and the state QueueManager
        currently is in, a status check will either report previous
        results, or perform a processResults operation (for failed tests).
        """
        for job_container in jobs:
            tester = job_container.getTester()

            # Determin if we can optimize speed by re-printing past results
            if self.__useImmediateStatus(job_container):
                self.harness.handleTestStatus(job_container)
                continue

            # jobs awaiting processResults (or previous finished non-passing tests we
            # wish to re-test)
            elif not self.downstreamNotFinished(job_container) and (tester.isWaiting() or tester.didFail()):
                # We need to adjust the current status to pending, so the testers will
                # consider testing the output.
                #
                # TODO: possibly change each tester to understand when to adjust their
                # own statuses
                tester.setStatus('processing results', tester.bucket_pending)

                # Perform processResults
                self.testOutput(job_container)

            # update the json session storage
            self.updateSessionData(job_container)

            # return the job to the TestHarness
            self.harness.handleTestStatus(job_container)

    def runJobs(self, jobs):
        """ Queue list of jobs to run """
        for job_container in jobs:
            # store launched jobs so we can use it when TestHarness calls waitFinish.
            self.__jobs.add(job_container)

            # Augment queue parameters
            template = self.augmentQueueParamsBase(job_container)

            # Prepare the worker directory
            self.__copyFiles(job_container, template)

            # Write the execution file
            self.__prepareQueueScript(template)

            # Save template information
            test_unique = self.getUnique(job_container)
            self.putData(test_unique,
                         queue_script=template['queue_script'],
                         working_dir=template['working_dir'],
                         job_name=template['job_name'])

            # Get derived launch command and launch this job (blocking)
            third_party_command = self.getQueueCommand(job_container)
            output = util.runCommand(third_party_command, cwd=template['working_dir'])

            # Call derived methods to ascertain job status
            self.handleJobStatus(job_container, output)

            # Print results
            self.queueJobs(status_jobs=[job_container])

            # Delete this job and get the next list of jobs
            job_dag = job_container.getDAG()
            job_dag.delete_node(job_container)
            next_job_group = self.getNextJobGroup(job_dag)

            # run these new jobs
            self.queueJobs(run_jobs=next_job_group)

    def testOutput(self, job_container):
        """
        Adjust the Tester for a proper working directory (QueueManager creates sub-directores),
        and allow derived Tester to perform processResults.

        Return resulting output.
        """
        tester = job_container.getTester()
        test_unique = self.getUnique(job_container)

        job = self.getData(test_unique, std_out=True, working_dir=True)
        stdout_file = os.path.join(job['working_dir'], job['std_out'])

        if os.path.exists(stdout_file):
            with open(stdout_file, 'r') as f:
                outfile = f.read()
        else:
            tester.setStatus('NO STDOUT FILE', tester.bucket_fail)
            return ''

        # Alter test_dir to reflect working_dir creation
        original_testdir = tester.getTestDir()
        tester.specs['test_dir'] = job['working_dir']

        if tester.hasRedirectedOutput(self.options):
            outfile += util.getOutputFromFiles(tester, self.options)

        # Allow the tester to verify its own output.
        output = tester.processResults(tester.specs['moose_dir'], self.options, outfile)

        # reset the original test_dir
        tester.specs['test_dir'] = original_testdir

        # Set the testers output with modifications made above
        job_container.setOutput(output)

        return output

    def handleJobStatus(self, job_container, output):
        """ Call derived methods for handling third party command output """
        return

    def postLaunch(self, jobs):
        """ Call dereived postLaunch operations """
        return

    def getUnique(self, job_container):
        """ Return unique identier for test """
        tester = job_container.getTester()
        return os.path.join(tester.getTestDir(), tester.getTestName())

    def checkStatusState(self):
        """ Return bool if we are processing status for launched jobs """
        return self.__status_check == True

    def downstreamNotFinished(self, job_container):
        """ Return bool for given job_container for all downstream jobs finished """
        job_dag = job_container.getOriginalDAG()
        downstream_jobs = job_dag.all_downstreams(job_container)

        for job in downstream_jobs:
            tester = job.getTester()
            if tester.didPass() or tester.isWaiting():
                continue
            return True

    def getData(self, test_unique, **kwargs):
        """
        Return requested information as a dictionary if available.

        Syntax:
          dict = getData(str(unique_test_name), **kwargs)

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

    def updateSessionData(self, job_container):
        """
        Populate the session storage with the current status of job_container
        """
        tester = job_container.getTester()
        unique_key = self.getUnique(job_container)

        self.putData(unique_key,
                     caveat_message=tester.getStatusMessage(),
                     status_bucket=dict(tester.getStatus()._asdict()))

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

    def getWorkingDir(self, job_container):
        """ Return the queue working directory for job """
        tester = job_container.getTester()
        return os.path.join(tester.getTestDir(), 'job_' + self.options.session_file)

    def augmentQueueParamsBase(self, job_container):
        """ Build the queue execution script """
        template = {}
        tester = job_container.getTester()

        for param in self.params.keys():
            template[param] = self.params[param]

        # Set CPU request count
        template['mpi_procs'] = tester.getProcs(self.options)

        # Set a path friendly job name
        template['job_name'] = ''.join(txt for txt in tester.specs['test_name'] if txt.isalnum() or txt in ['_', '-'])

        # Set working directory
        template['working_dir'] = self.getWorkingDir(job_container)

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
        template = self.augmentQueueParams(job_container, template)

        return template

    def waitFinish(self):
        """
        Inherited method called once all jobs have been sent to queueJobs.
        QueueManager uses this method as a means to trigger other post-operation
        methods after _all_ jobs have been launched. Such as calling the
        queueJobs method a second time to perform status checks. As well as
        writing the finalized json session file.
        """

        # Set the status_check switch
        if self.__status_check == False:
            self.__status_check = True

            # TestHarness is finished sending us jobs, do postLaunch stuff (for PBS this
            # means releasing the jobs using qrls)
            self.postLaunch(self.__jobs)

            # Inform the user of our status change
            print('\nJobs launched. Begin checking status...')

            # Call queueJobs again using the populated set of jobs as our list (clever!
            # no longer do we need to nerf the TestHarness's os.walk routine...)
            self.queueJobs(status_jobs=self.__jobs)

        # Write session data to file
        self.writeSessionFile()

    def queueJobs(self, status_jobs=[], run_jobs=[]):
        """
        Entry point into the QueueManager. Supply queueJobs with a list
        of jobs to either launch, or check statuses for.

        Syntax:

           Launch job:
            queueJobs(run_jobs=[list of jobs])

           Print status:
            queueJobs(status_jobs=[list of jobs])
        """

        # if __status_check is True, we do not care which 'queue' might have tried to initiate.
        # QueueManager knows best when this is True.
        if self.__status_check:
            # Collect all jobs no matter the queue other schedulers are attempting to use.
            # __status_check is True, so QueueManager knows best ATM.
            all_jobs = set(status_jobs).union(run_jobs)
            self.processJobs(all_jobs)

        # NOTE: The Scheduler attempting to queue a 'status_jobs' while __status_check is False
        # means this test is one of the skipped variety
        elif status_jobs:
            self.statusJobs(status_jobs)

        elif run_jobs:
            self.runJobs(run_jobs)
