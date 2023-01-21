#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys, os, json, shutil
from collections import namedtuple
from Scheduler import Scheduler
from TestHarness.StatusSystem import StatusSystem # Determin previous status

class QueueManager(Scheduler):
    """
    QueueManager is a Scheduler plugin responsible for allowing the testers to be scheduled via a
    third-party queue system (like PBS).

    The QueueManager works by intercepting and altering the statuses of all but one job contained
    in the group to a finished state. This affords us the behavior of only using the runner thread
    pool once per group (see augmentJobs).

    Using this one unmodified job, the spec file involved is noted, and instructs the derived
    scheduler how to launch this one single spec file (using --spec-file), along with any
    supplied/allowable command line arguments (--re, --cli-args, --ignore, etc).

    The third-party queueing manager then executes `run_tests --spec-file /path/to/spec_file`.

    It is the results of this additional ./run_tests run, that is captured and presented to the user as
    the finished result of the test.
    """
    @staticmethod
    def validParams():
        params = Scheduler.validParams()
        return params

    def __init__(self, harness, params):
        Scheduler.__init__(self, harness, params)
        self.harness = harness
        self.options = self.harness.getOptions()
        self.__job_storage_file = self.harness.original_storage
        self.__clean_args = None
        self.__status_system = StatusSystem()

    def augmentJobs(self, Jobs):
        """
        Filter through incomming jobs and figure out if we are launching them
        or attempting to process finished results.
        """
        if self.options.dry_run:
            return
        # Flatten the DAG. We want to easily iterate over all jobs produced by the spec file
        Jobs.removeAllDependencies()

        # Perform cleanup operations and return if thats what the user wants
        if self.options.queue_cleanup:
            self._cleanupFiles(Jobs)
            return

        # Create a namedtuple of frequently used information contained within Jobs, so we can
        # more easily pass this information among our methods
        job_list = Jobs.getJobs()
        if job_list:
            queue_data = namedtuple('JobData', ['jobs', 'job_dir', 'json_data', 'plugin'])
            job_data = queue_data(jobs=Jobs,
                                  job_dir=job_list[0].getTestDir(),
                                  json_data=self.options.results_storage,
                                  plugin=self.harness.scheduler.__class__.__name__)

            if self._isProcessReady(job_data):
                self._setJobStatus(job_data)

            elif self._isLaunchable(job_data):
                self._prepareJobs(job_data)

    def createQueueScript(self, job, template):
        """ Write the launch script to disc """
        if self.options.dry_run:
            return
        # Get a list of prereq tests this test may have
        try:
            with open(self.params['queue_template'], 'r') as f:
                content = f.read()

                with open(template['launch_script'], 'w') as queue_script:

                    # Do all of the replacements for valid parameters
                    for key in template.keys():
                        if key.upper() in content:
                            content = content.replace('<' + key.upper() + '>', str(template[key]))

                    # Strip out parameters that were not supplied
                    for key in template.keys():
                        if key.upper() not in content:
                            content = content.replace('<' + key.upper() + '>', '')

                    queue_script.write(content)
        except IOError as e:
            print(e)
            sys.exit(1)

    def reserveSlots(self, job, j_lock):
        """
        Inherited method from the Scheduler to handle slot allocation.
        QueueManager does not need an allocation system, so this method simply returns True
        """
        return True

    def getBadArgs(self):
        """ Arguments which should be removed from the launch script invoking ./run_tests """
        return []

    def getBadKeyArgs(self):
        """ Key/Value arguments which should be removed from the launch script invoking ./run_tests """
        return []

    def getCores(self, job_data):
        """ iterate over Jobs and collect the maximum core requirement from the group of jobs which will run """
        slots = 1
        for job in [x for x in job_data.jobs.getJobs() if not x.isSkip()]:
            slots = max(slots, job.getSlots())

        return slots

    def getMaxTime(self, job_data):
        """ iterate over Jobs and increment the total allowable time needed to complete the entire group """
        total_time = 0
        for job in [x for x in job_data.jobs.getJobs() if not x.isSkip()]:
            total_time += int(job.getMaxTime())

        return total_time

    def addDirtyFiles(self, job, file_list=[]):
        """ append list of files which will be generated by derived scheduler """
        _dirty_files = self.getDirtyFiles(job)
        file_list.extend(_dirty_files)
        file_list = list(set(file_list))
        job.addMetaData(DIRTY_FILES=file_list)

    def getDirtyFiles(self, job):
        """ return list of files not indigenous to the repository which was created by third party schedulers """
        return job.getMetaData().get('DIRTY_FILES', [])

    def cleanAndModifyArgs(self):
        """
        Filter out any arguments that will otherwise break the TestHarness when launched _within_
        the third party scheduler (such as --pbs)
        """
        # return cached args if we have already produced clean args
        if not self.__clean_args:
            current_args = list(sys.argv[1:])

            # Ask derived schedulers for any additional args we should strip from sys.args
            bad_args = self.getBadArgs()
            bad_keyword_args = self.getBadKeyArgs()

            # Split keyword args so we can match/remove them (the key, and its value pair)
            key_value_args = [x for x in current_args if '=' in x]
            for arg in key_value_args:
                current_args.remove(arg)
                current_args.extend(arg.split('='))

            # Note: we are removing cli-args/ignore because we need to re-encapsulate them below
            bad_keyword_args.extend(['--spec-file', '-i', '--cli-args', '-j', '-l', '-o', '--output-dir', '--ignore', '--re'])

            # remove the key=value pair argument
            for arg in bad_keyword_args:
                if arg in current_args:
                    key = current_args.index(arg)
                    del current_args[key:key+2]

            # Special: re-encapsulate --cli-args
            if self.options.cli_args:
                current_args.extend(['--cli-args', '"%s"' % self.options.cli_args])
            if self.options.ignored_caveats:
                current_args.extend(['--ignore', '"%s"' % self.options.ignored_caveats])
            if self.options.reg_exp:
                current_args.extend(['--re', '"%s"' % self.options.reg_exp])

            # remove any specified positional arguments
            for arg in bad_args:
                if arg in current_args:
                    current_args.remove(arg)

            self.__clean_args = current_args

        return self.__clean_args

    def getRunTestsCommand(self, job, cpus):
        """ return the command necessary to launch the TestHarness within the third party scheduler """

        # Build ['/path/to/run_tests', '-j', '#']
        command = [os.path.join(self.harness.run_tests_dir, 'run_tests'),
                   '-j', str(cpus)]

        # get current sys.args we are allowed to include when we launch run_tests
        args = list(self.cleanAndModifyArgs())

        # Build [<args>, '--spec-file' ,/path/to/tests', '-o', '/path/to', '--sep-files']
        args.extend(['--spec-file',
                     os.path.join(job.getTestDir(),
                     self.options.input_file_name),
                     '-o', job.getTestDir(),
                     '--sep-files'])

        # Build [<command>, <args>]
        command.extend(args)

        return command

    def hasQueuingFailed(self, job_data):
        """ Determine if the third party scheduler killed the job prematurely """
        return False

    def _isProcessReady(self, job_data):
        """
        Return bool on `run_tests --spec_file` submission results being available. Due to the
        way the TestHarness writes to this results file (when the TestHarness exits), this file,
        when available, means every test contained therein is finished in some form or another.

        If the result file does not exist, determine if it ever will exist. Tests which can fall
        into this group, are those which were: skipped, deleted, silent, etc during the initial
        launch phase.
        """
        # No session file. Return immediately.
        if not job_data.json_data.get(job_data.job_dir, False):
            return False

        is_ready = True
        # Job group exists in queue session and was apart of the queueing process
        job_meta = job_data.json_data.get(job_data.job_dir, {})
        scheduler = job_data.json_data.get('SCHEDULER', '')
        if job_meta:
            # result file exists (jobs are finished)
            if os.path.exists(os.path.join(job_data.job_dir, self.__job_storage_file)):
                pass

            # ask derived scheduler if this job has failed
            elif self.hasQueuingFailed(job_data):
                for job in job_data.jobs.getJobs():
                    job.setStatus(job.error)
                is_ready = False

            # result does not yet exist but will in the future
            else:
                for job in job_data.jobs.getJobs():
                    tester = job.getTester()
                    if tester.isSilent():
                        continue

                    status, message, caveats = job.previousTesterStatus(self.options, job_data.json_data)
                    if status == self.__status_system.skip or status == self.__status_system.no_status:
                        tester.setStatus(status, 'SKIP')
                    else:
                        tester.setStatus(status, job_meta[scheduler]['STATUS'])
                    if caveats:
                        tester.addCaveats(caveats)
                    status_message = tester.getStatusMessage()

                    # This single job will enter the runner thread pool
                    if status_message == "LAUNCHING":
                        tester.setStatus(tester.queued)

                is_ready = False

        # Job group not originally launched
        else:
            for job in job_data.jobs.getJobs():
                tester = job.getTester()
                status, message, caveats = job.previousTesterStatus(self.options, job_data.json_data)
                tester.setStatus(status, message)
                if caveats:
                    tester.addCaveats(caveats)

                if tester.isNoStatus():
                    tester.setStatus(tester.silent)
            is_ready = False

        if not is_ready:
            for job in job_data.jobs.getJobs():
                job.setStatus(job.finished)

        return is_ready

    def _isLaunchable(self, job_data):
        """ bool if jobs are ready to launch """
        # results data exists (set during scheduler plugin initialization), so do no launch again
        if job_data.json_data.get(job_data.job_dir, False):
            return False

        return True

    def _prepareJobs(self, job_data):
        """
        Prepare jobs for launch.

        Grab an arbitrary job and record any necessary information the third party
        queueing systems requires for launch (walltime, ncpus, etc). Set all other
        jobs to a finished state. The arbitrary job selected will be the only job
        which enters the runner thread pool, and executes the commands neccessary
        for job submission.
        """
        job_list = job_data.jobs.getJobs()

        # Clear any caveats set (except skips). As they do not apply during job submission
        for job in [x for x in job_list if not x.isSkip()]:
            job.clearCaveats()

        if job_list:
            launchable_jobs = [x for x in job_list if not x.isFinished()]
            if launchable_jobs:
                executor_job = job_list.pop(job_list.index(launchable_jobs.pop(0)))
                scheduler_meta = {job_data.plugin : {'QUEUEING_NCPUS'   : self.getCores(job_data),
                                                     'QUEUEING_MAXTIME' : self.getMaxTime(job_data)}
                                 }
                self.options.results_storage[executor_job.getTestDir()] = scheduler_meta

                executor_job.setStatus(executor_job.hold)
                for job in launchable_jobs:
                    tester = job.getTester()
                    tester.setStatus(tester.queued, 'LAUNCHING')
                    job.setStatus(job.finished)

    def _prevJobGroupFinished(self, jobs):
        """ Loop through jobs and return immediately if any one job has a finished status """
        for job in jobs:
            # ignore detection of skipped/silent/deleted finished statuses.
            if job.isSilent() or job.isSkip():
                continue
            (key, value) = job.getTestDir(), job.getTestName()
            previous_status = self.__status_system.createStatus(self.options.results_storage[key][value]['STATUS'])
            if (self.__status_system.isValid(previous_status)
                and previous_status not in self.__status_system.getPendingStatuses()):
                return True
        return False

    def _setJobStatus(self, job_data):
        """
        Read the json results file for the finished submitted job group, and match our
        job statuses with the results found therein.
        """
        job_list = job_data.jobs.getJobs()
        if job_list:
            testdir_json = os.path.join(job_data.job_dir, self.__job_storage_file)

            with open(testdir_json, 'r') as f:
                try:
                    # Determine if we've already recorded the results for this job group
                    if self._prevJobGroupFinished(job_list):
                        results = self.options.results_storage
                    else:
                        results = json.load(f)
                except ValueError:
                    print('Unable to parse json file: %s' % (testdir_json))
                    sys.exit(1)

            group_results = results[job_data.job_dir]

            # Continue to store previous third-party queueing data
            job_meta = self.options.results_storage[job_data.job_dir]
            job_list[0].addMetaData(**{job_data.plugin : job_meta[job_data.plugin]})
            job_meta[job_data.plugin]['STATUS'] = 'FINISHED'

            for job in job_list:
                # Perhaps the user is filtering this job (--re, --failed-tests, etc)
                tester = job.getTester()
                job.setStatus(job.finished)
                if tester.isSilent() or tester.isSkip():
                    continue
                elif self.options.failed_tests and tester.isPass():
                    tester.setStatus(tester.silent)
                    continue

                if group_results.get(job.getTestName(), {}):
                    job_results = group_results[job.getTestName()]
                    status, message, caveats = job.previousTesterStatus(self.options, results)
                    tester.setStatus(status, message)
                    if caveats:
                        tester.addCaveats(caveats)

                    # Recover useful job information from job results
                    job.setPreviousTime(job_results['TIMING'])

                    # Read output file (--sep-files-ok|fail)
                    if job.getOutputFile() and os.path.exists(job.getOutputFile()):
                        self.addDirtyFiles(job, [job.getOutputFile()])
                        if (self.options.reg_exp
                            or self.options.failed_tests
                            or self.options.verbose) and not self.options.quiet:
                            with open(job.getOutputFile(), 'r') as outfile:
                                job.setOutput(outfile.read())
                        else:
                            job.setOutput(f'See error in file: {job.getOutputFile()}')
                    else:
                        job.setOutput('Output file is not available, or was never generated')

                # This is a newly added test in the spec file, which was not a part of original launch
                else:
                    tester.addCaveats('not originally launched')
                    tester.setStatus(tester.skip)

    def _setSilentForClean(self, Jobs):
        """ silence and set a finished status for all testers """
        job_list = Jobs.getJobs()
        for job in job_list:
            tester = job.getTester()
            tester.setStatus(tester.silent)
            job.setStatus(job.finished)
        return job_list

    def _cleanupFiles(self, Jobs):
        """ perform cleanup operations """
        job_list = self._setSilentForClean(Jobs)
        top_job_key = job_list[0].getTestDir()
        plugin = self.harness.scheduler.__class__.__name__

        # Top Job (entire TestDir group) not originally part of what was launched
        # (not launched due to: --re, -i --spec-file)
        if top_job_key not in self.options.results_storage.keys():
            return
        # All jobs ended up being skipped in this group
        # (compiler!=gcc, heavy, petsc_version, etc)
        elif plugin not in self.options.results_storage[top_job_key].keys():
            return

        # Build file_list with files we should delete
        file_list = [os.path.join(top_job_key, self.options.results_file)]
        job_meta = self.options.results_storage[top_job_key]
        scheduler_meta = job_meta[plugin]
        file_list.extend(scheduler_meta.get('DIRTY_FILES', []))
        for test_dir_key, meta in job_meta.items():
            if type(meta) == type({}) and 'META_DATA' in meta.keys():
                file_list.extend(meta["META_DATA"].get("DIRTY_FILES", []))

        # Delete files generated by jobs and the scheduler
        for dirty_file in file_list:
            # Safty check. Any indigenous file generated by QueueManager should only exist in the tester directory
            if os.path.dirname(dirty_file) == top_job_key:
                try:
                    if os.path.isdir(dirty_file):
                        shutil.rmtree(dirty_file)
                    else:
                        os.remove(dirty_file)
                except OSError:
                    pass
