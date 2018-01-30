#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from QueueManager import QueueManager
import os, re

## This Class is responsible for maintaining an interface to the PBS scheduling syntax
class RunPBS(QueueManager):
    @staticmethod
    def validParams():
        params = QueueManager.validParams()
        params.addParam('queue_template', os.path.join(os.path.abspath(os.path.dirname(__file__)), 'pbs_template'), "Location of the PBS template")
        return params

    def __init__(self, harness, params):
        QueueManager.__init__(self, harness, params)
        self.params = params

    def postLaunchCommand(self, jobs):
        """ Release supplied jobs that should be on hold """
        job_ids = []
        for job in jobs:
            job_data = self.getData(job.getUniqueIdentifier(), job_id=True)
            job_ids.append(job_data['job_id'])

        if job_ids:
            return 'qrls %s' % (' '.join(job_ids))

    def handleJobStatus(self, job, output):
        """
        Call appropriate methods depending on current state of QueueManager
        and return a resulting dictionary of information relevant to PBS
        """
        if self.checkStatusState():
            return self.handleQstatOutput(job, output)
        else:
            return self.handleQsubOutput(job, output)

    def getQueueCommand(self, job):
        """ Return appropriate PBS command depending on current QueueManager state """
        test_unique = job.getUniqueIdentifier()

        if self.checkStatusState():
            job = self.getData(test_unique, job_id=True)
            return 'qstat -xf %s' % (job['job_id'])
        else:
            job = self.getData(test_unique, queue_script=True, working_dir=True)
            return 'qsub -h %s' % (os.path.join(job['working_dir'], job['queue_script']))

    def augmentQueueParams(self, job, template):
        """ Populate QSUB template with relevant PBS information for job """
        tester = job.getTester()

        # Discover prereq launch job IDs
        dag_obj = job.getOriginalDAG()
        prereqs = dag_obj.predecessors(job)

        template['prereq'] = ''
        if prereqs:
            prereq_job_ids = []
            for prereq in prereqs:
                unique = prereq.getUniqueIdentifier()
                tmp_json = self.getData(unique, job_id=True)
                prereq_job_ids.append(tmp_json['job_id'])
            template['prereq'] = '#PBS -W depend=afterany:%s' % (':'.join(prereq_job_ids))
            template['prereq_ids'] = prereq_job_ids

        # Convert MAX_TIME to hours:minutes for walltime use
        hours = int(int(tester.specs['max_time']) / 3600)
        minutes = int(int(tester.specs['max_time']) / 60) % 60
        template['walltime'] = '{0:02d}'.format(hours) + ':' + '{0:02d}'.format(minutes) + ':00'

        # Add PBS project directive
        if self.options.queue_project:
            template['pbs_project'] = '#PBS -P %s' % (self.options.queue_project)

        # Set node placement
        template['place'] = 'free'

        # Combined stdout and stderr into one stream
        template['combine_streams'] = '#PBS -j oe'

        return template

    def handleQsubOutput(self, job, output):
        """
        Set the job's status and return any relevant information
        about the launched PBS job we want stored in QueueManager's
        session storage file.
        """
        pattern = re.compile(r'^(\d+)\.[\W\w]+$')
        tester = job.getTester()

        job_info = {}
        if pattern.search(output):
            job_id = pattern.search(output).group(1)
            with self.dag_lock:
                tester.setStatus('LAUNCHED %s' %(str(job_id)), tester.bucket_queued)
                session_data = self.getData(job.getUniqueIdentifier(), job_name=True)

            job_info = { 'job_id' : job_id, 'std_out' : session_data['job_name'] + '.o' + job_id }

        # Failed to launch somehow. Set the tester output to command output. Hopefully something
        # useful in there to display to the user on why we failed
        else:
            job.setOutput(output)
            with self.dag_lock:
                tester.setStatus('QSUB FAILURE', tester.bucket_fail)

        return job_info

    def handleQstatOutput(self, job, output):
        """
        Set the job's status and return any relevant information
        about the pending PBS job we want stored in QueueManager's
        session storage file.
        """
        tester = job.getTester()
        job_state = re.search(r'job_state = (\w)', output)

        # Default bucket
        bucket = tester.bucket_queued

        if job_state:
            # Job is finished
            if job_state.group(1) == 'F':
                # The exit code PBS recorded when the application exited
                tester.exit_code = re.search(r'Exit_status = (\d+)', output).group(1)

                # non-zero exit codes are handled properly elsewhere (as a CRASH). So do not set any
                # failed bucket here. Instead, add a message (caveat) as to why it crashed (if we know).
                # additional info in next TL;DR note.

                # Set the bucket that allows processResults to commence
                reason = 'WAITING'
                bucket = tester.bucket_waiting_processing

                # NOTE:
                # Setting a failed bucket in RunPBS derived scheduler will cause the TestHarness to assume
                # there is no output from the job we launched. but we do have output in this case. And it
                # contains valuable information as to why PBS killed the job. So just set a caveat. The
                # TestHarness _will_ fail the job later, correctly, because of the non-zero exit code.
                if tester.exit_code == '271':
                    tester.addCaveats('Killed by PBS')

            # Job is currently running
            elif job_state.group(1) == 'R':
                reason = 'RUNNING'

            # Job is exiting
            elif job_state.group(1) == 'E':
                reason = 'EXITING'

            # Job is currently queued
            elif job_state.group(1) == 'Q':
                reason = 'QUEUED'

            # Job is waiting for other jobs
            elif job_state.group(1) == 'H':
                reason = 'HOLDING'

            # Unknown statuses should be treated as failures
            else:
                reason = 'UNKNOWN PBS STATUS'
                bucket = tester.bucket_fail
                job.setOutput(output)

        else:
            # Job status not available
            reason = 'INVALID QSTAT RESULTS'
            bucket = tester.bucket_fail
            job.setOutput(output)

        with self.dag_lock:
            tester.setStatus(reason, bucket)

        return {}
