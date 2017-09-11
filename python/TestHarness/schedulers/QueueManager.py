from Scheduler import Scheduler
from collections import namedtuple
import os, re, json, shutil, errno

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

    def __getStatusBucket(self, test_unique):
        """ Private method used to create a compatible status bucket """
        data = self.getData(test_unique, status_bucket=True)
        caveat = data['status_bucket']['status']
        caveat_color = data['status_bucket']['color']
        tmp_bucket = namedtuple('test_status', data['status_bucket'])

        return tmp_bucket(status=caveat, color=caveat_color)

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

    def postLaunch(self, jobs):
        """ Call dereived postLaunch operations """
        return

    def getUnique(self, job_container):
        """ Return unique identier for test """
        tester = job_container.getTester()
        return os.path.join(tester.getTestDir(), tester.getTestName())

    def checkJob(self, job_container):
        """ Call derived checkJob method """
        return

    def launchJob(self, job_container):
        """ Call derived launchJob method """
        return

    def checkStatusState(self):
        """ Return bool if we are processing status for launched jobs """
        return self.__status_check == True

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

    def getWorkingDir(self, job_container):
        """ Return the queue working directory """
        tester = job_container.getTester()
        return os.path.join(tester.getTestDir(), 'job_' + self.options.session_file)

    def writeSessionFile(self):
        """ Write the contents of your session to the session file """
        self.__session_file.seek(0)
        json.dump(self.__session_data, self.__session_file, indent=2)
        self.__session_file.truncate()
        self.__session_file.close()

    def waitFinish(self):
        """
        Inherited method called once all jobs have been sent to queueJobs.
        QueueManager uses this method as means to trigger other post-operation
        methods. Such as calling the queueJobs method a second time to perform
        status checks. As well as writing the finalized json session file.
        """

        # Set the status_check switch
        if self.__status_check == False:
            self.__status_check = True

            # TestHarness is finished sending us jobs, do postLaunch stuff (for PBS this
            # means releasing the jobs using qrls)
            self.postLaunch(self.__jobs)

            # Inform the user of our status change
            print '\nJobs launched. Begin checking status...'

            # Call queueJobs again using the populated set of jobs as our list (clever!
            # no longer do we need to nerf the TestHarness's os.walk routine...)
            self.queueJobs(status_jobs=self.__jobs)

        # Write session data to file
        self.writeSessionFile()

    def updateSessionData(self, job_container):
        """
        Populate the session storage with the current status of job_container
        """
        tester = job_container.getTester()
        unique_key = self.getUnique(job_container)

        self.putData(unique_key,
                     caveat_message=tester.getStatusMessage(),
                     isFinished=tester.isFinished(),
                     didPass=tester.didPass(),
                     didFail=tester.didFail(),
                     isSkipped=tester.isSkipped(),
                     status_bucket=dict(tester.getStatus()._asdict()))

    def copyFiles(self, job_container):
        """ Copy the necessary test files the job will need in order to execute """
        tester = job_container.getTester()
        test_unique = self.getUnique(job_container)

        json_data = self.getData(test_unique, template_data=True)
        template = json_data['template_data']

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

    def downstreamFinished(self, job_container):
        """ Return bool for given job_container for all downstream jobs finished """
        job_dag = job_container.getOriginalDAG()
        downstream_jobs = job_dag.all_downstreams(job_container)
        for job in downstream_jobs:
            test_unique = self.getUnique(job)
            json_job = self.getData(test_unique, isFinished=True)

            # None would mean the upstream job never launched
            if 'isFinished' not in json_job.keys() or json_job['isFinished'] is False:
                return False
        return True

    def queueJobs(self, status_jobs=[], run_jobs=[]):
        """
        Entry point into the QueueManager. Supply queueJobs with a list
        of jobs to either launch, or print statuses for.

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

            launched_dags = []
            for job_container in all_jobs:
                test_unique = self.getUnique(job_container)

                # Only interested in jobs contained in the current session
                if test_unique not in self.__session_data.keys():
                    continue

                # Yo dawg, I heard you like DAGs
                job_dag = job_container.getDAG()
                original_dag = job_container.getOriginalDAG()
                reverse_dag = original_dag.reverse_edges()

                # job_dag is unique and shared amongst a group of job_containers. We only want
                # to work with one of these DAGs per group. So this is how we do it.
                if job_dag in launched_dags:
                    continue
                launched_dags.append(job_dag)

                # Using the original DAG the job_container group began with, and a reverse clone
                # of that to boot, itereate _backwards_ to try and run postProcess on TestB,
                # before running postProcess on TestA (because performing postProcess on TestA,
                # while TestB might still be _running_, can cause bad things for some tests.
                for original_job_container in reverse_dag.topological_sort():
                    self.checkStatus(original_job_container)

        # NOTE: The Scheduler attempting to queue a 'status_jobs' while __status_check is False
        # means this test is one of the skipped variety
        elif status_jobs:
            for job_container in status_jobs:

                # We do not want to save silent tests to the session file
                tester = job_container.getTester()
                if tester.isSilent():
                    return

                self.checkStatus(job_container)

        elif run_jobs:
            for job_container in run_jobs:
                # store launched jobs so we can use it when TestHarness calls waitFinish.
                self.__jobs.add(job_container)

                # call derived launchJob methods
                self.launchJob(job_container)

                # send this job to the status job queue
                self.queueJobs(status_jobs=[job_container])

                job_dag = job_container.getDAG()

                # Delete this job from the DAG
                job_dag.delete_node(job_container)

                # Get next job list
                next_job_group = self.getNextJobGroup(job_dag)

                # run these new jobs
                self.queueJobs(run_jobs=next_job_group)

    def checkStatus(self, job_container):
        """
        Check and print the status of supplied job_container. This method will also
        update the session data with the new status of job_container.
        """
        tester = job_container.getTester()
        test_unique = self.getUnique(job_container)

        # Optimize the status check by determining if we can re-use a status in the session data.
        # This allows us to avoid calling third party scheduling binaries, postProcess exodiff, etc
        #
        # The only caveat here, is the queued verses pending status. The testers, during postProcess
        # operations assume if they are anything but pending after they are finished, then they are
        # _not_ passing and thus not set a new status. So, we need to adjust the Queued status to be
        # Pending just for the sake of status checks on jobs that still need to have their
        # postProcess operations done on them. I don't like it. But someone, needs to be the ugly
        # duckling and hack this conversion.
        #
        # TODO: I _think_ if we change each testers logic to:  if not self.didFail() that may
        #       suffice?
        if self.__status_check:
            status_bucket = self.__getStatusBucket(test_unique)
            caveat = self.getData(test_unique, caveat_message=True)

            # Read huge note above.
            if status_bucket == tester.bucket_queued:
                # Make the status pending instead, so the tester can adjust the status to a passing
                # test once its postProcess is complete. (see any tester fo more details: Exodiff.py)
                tester.setStatus(caveat['caveat_message'], tester.bucket_pending)
            else:
                tester.setStatus(caveat['caveat_message'], status_bucket)

        # Print previous results if test passed
        if tester.didPass():
            self.harness.handleTestStatus(job_container)

        elif self.__status_check and test_unique in self.__session_data.keys():
            # perform _downstream_ dependency checks when checking statuses
            if self.downstreamFinished(job_container):

                # call derived checkJob methods (processResults)
                self.checkJob(job_container)

            # Downstream tests not yet complete
            else:
                tester.setStatus('WAITING', tester.bucket_queued)

            # print updated status
            self.harness.handleTestStatus(job_container)

        else:
            # print updated status
            self.harness.handleTestStatus(job_container)

        # update the session data
        self.updateSessionData(job_container)
