from QueueManager import QueueManager
import os, re
from TestHarness import util

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

    def postLaunch(self, jobs):
        """ Release jobs that were placed on hold """
        print('releasing jobs...')
        # TODO: Do something else, than this one-at-a-time stuff.
        #       Also we should be returning a command to run, and not calling runCommand ourselves.
        for job_container in jobs:
            test_unique = self.getUnique(job_container)
            json_session = self.getData(test_unique, job_id=True)
            util.runCommand('qrls %s' % (json_session['job_id']))

    def handleJobStatus(self, job_container, output):
        """ Call appropriate methods depending on current state of QueueManager """
        if self.checkStatusState():
            return self.handleQstatOutput(job_container, output)
        else:
            return self.handleQsubOutput(job_container, output)

    def getQueueCommand(self, job_container):
        """ Return appropriate PBS command depending on current QueueManager state """
        test_unique = self.getUnique(job_container)

        if self.checkStatusState():
            job = self.getData(test_unique, job_id=True)
            return 'qstat -xf %s' % (job['job_id'])
        else:
            job = self.getData(test_unique, queue_script=True, working_dir=True)
            return 'qsub -h %s' % (os.path.join(job['working_dir'], job['queue_script']))

    def augmentQueueParams(self, job_container, template):
        """ Populate QSUB template with relevent PBS information for job """
        tester = job_container.getTester()

        # Discover prereq launch job IDs
        prereqs = tester.getPrereqs()
        template['prereq'] = ''
        if prereqs:
            prereq_job_ids = []
            for prereq_test_name in prereqs:
                pre_unique = os.path.join(tester.getTestDir(), prereq_test_name)
                tmp_json = self.getData(pre_unique, job_id=True)
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

    def handleQsubOutput(self, job_container, output):
        """ Set a status based on output supplied by qsub """
        tester = job_container.getTester()
        test_unique = self.getUnique(job_container)
        pattern = re.compile(r'^(\d+)\.[\W\w]+$')

        if pattern.search(output):
            job_id = pattern.search(output).group(1)

            # Update the queue_data based on results
            job = self.getData(test_unique, job_name=True)

            self.putData(test_unique, job_id=job_id, std_out=job['job_name'] + '.o%s' %(job_id))

            # It may seem odd to place a tester that we know is queued, into a
            # TestHarness pending status. But we do so because at this stage, the
            # TestHarness is on its first-half stage, so it truly is still _pending_.
            # It still needs to attempt processResults (second-half). Which when it
            # does, will either see this test as awaiting_processing (and end with
            # pass or fail) or finished as queued because the test has not yet
            # finished running in the PBS system.

            # See handleQstatOutput below for more detail. Consider that method the
            # second-half stage.
            tester.setStatus('%s LAUNCHED' % (str(job_id)), tester.bucket_pending)

        elif 'command not found' in output:
            tester.setStatus('QSUB NOT FOUND', tester.bucket_fail)

        else:
            tester.setStatus('QSUB INVALID RESULTS: %s' % (output), tester.bucket_fail)

        return output

    def handleQstatOutput(self, job_container, output):
        """ Handle statuses supplied by qstat output. """
        tester = job_container.getTester()
        output_value = re.search(r'job_state = (\w)', output)

        if output_value:
            # Job is finished
            if output_value.group(1) == 'F':
                reason = 'WAITING'

                # Tester has finished running in the PBS system.
                #
                # Note: Other testers may still be running that can influence
                # the results of this tester.
                tester.setStatus(reason, tester.bucket_waiting_processing)

            # Job is currently running
            elif output_value.group(1) == 'R':
                reason = 'RUNNING'

            # Job is exiting
            elif output_value.group(1) == 'E':
                reason = 'EXITING'

            # Job is currently queued
            elif output_value.group(1) == 'Q':
                reason = 'QUEUED'

            # Job is waiting for other jobs
            elif output_value.group(1) == 'H':
                reason = 'HOLDING'

            # Unknown statuses should be treated as failures
            else:
                reason = 'UNKNOWN PBS STATUS'
                tester.setStatus(reason, tester.bucket_fail)

            # Update the tester caveat and adjust the tester for a finished status
            if tester.isPending():
                tester.setStatus(reason, tester.bucket_queued)

        else:
            # Job status not available
            reason = 'INVALID QSTAT RESULTS'
            tester.setStatus(reason, tester.bucket_fail)

        return output
