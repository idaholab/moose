from timeit import default_timer as clock
from MooseObject import MooseObject
from QueueManager import QueueManager
import os, re
## This class launches jobs using the PBS queuing system
class RunPBS(QueueManager):
    @staticmethod
    def validParams():
        params = QueueManager.validParams()
        params.addRequiredParam('scheduler', 'RunPBS',       "the name of the scheduler used")
        params.addRequiredParam('place',       'free',       "node placement")
        params.addRequiredParam('walltime',        '',       "amount of time to request for this test")

        params.addParam('combine_streams',  '#PBS -j oe',           "combine stdout and stderr")
        params.addParam('pbs_queue',                  '',           "the PBS queue to use")
        params.addParam('pbs_project',                '',           "the PBS project to use")
        params.addParam('prereq',                     '',           "list of jobs this job depends on")

        return params

    def __init__(self, harness, params):
        QueueManager.__init__(self, harness, params)
        self.specs = params
        self.launched_ids = []

    # Return command to release all launched jobs
    def getpostQueueCommand(self):
        if len(self.launched_ids):
            print '\nreleasing launched jobs...'
            return 'qrls ' + ' '.join(self.launched_ids)

    # Return command necessary for current mode
    def getQueueCommand(self, tester):
        if self.options.processingQueue:
            return 'qstat -xf %s' % (self.getQueueID(tester))
        else:
            return 'qsub -h %s' % (self.getQueueScript(tester))

    # Read the template file make some changes, and write the launch script
    def prepareQueueScript(self, tester, preq_list):
        # Add prereq job id, now that we know them!
        if len(preq_list):
            self.specs['prereq'] = '#PBS -W depend=afterany:%s' % (':'.join(preq_list))

    # Augment PBS Queue specs for tester params
    def updateParams(self, tester):
        # Use the PBS template
        self.specs['template_script'] = os.path.join(tester.specs['moose_dir'], 'python/TestHarness/schedulers', 'pbs_template')

        # Convert MAX_TIME to hours:minutes for walltime use
        hours = int(int(tester.specs['max_time']) / 3600)
        minutes = int(int(tester.specs['max_time']) / 60) % 60
        self.specs['walltime'] = '{0:02d}'.format(hours) + ':' + '{0:02d}'.format(minutes) + ':00'

        # Add PBS project directive
        if self.options.project:
            self.specs['pbs_project'] = '#PBS -P %s' % (self.options.project)

        # Add PBS queue directive
        if self.options.queue:
            self.specs['pbs_queue'] = '#PBS -q %s' % (self.options.queue)

    # Handle statuses supplied by qsub output
    def handleQueueLaunch(self, tester, output):
        pattern = re.compile(r'^(\d+)\.[\W\w]+$')

        if pattern.search(output):
            job_id = pattern.search(output).group(1)
            # Update the queue_data with launch information
            self.updateQueueFile(tester, job_id)

            # Append this job_id to a self governed list of launched jobs
            self.launched_ids.append(job_id)

            # Set the tester status
            tester.setStatus('%s LAUNCHED' % (str(job_id)), tester.bucket_pending)

        elif 'command not found' in output:
            tester.setStatus('QSUB NOT FOUND', tester.bucket_fail)

        else:
            tester.setStatus('QSUB INVALID RESULTS: %s' % (output), tester.bucket_fail)

        return output

    # Handle statuses supplied by qstat output
    def handleQueueStatus(self, tester, qstat_output):
        output_value = re.search(r'job_state = (\w)', qstat_output)
        exit_code = re.search(r'Exit_status = (-?\d+)', qstat_output)
        if exit_code:
            exit_code = int(exit_code.group(1))

        # Set the initial reason for a test NOT to be finished
        reason = ''

        # For all results excect Finished, return actual qstat output (useful for --verbose output)
        output = qstat_output

        if output_value:
            # Report the current status of JOB_ID
            if output_value.group(1) == 'F':
                test_info = self.getQueueData(tester)
                stdout_file = os.path.join(test_info['test_dir'], test_info['job_name'] + '.o%s' % (test_info['id']))

                # Read the stdout file and allow testOutputAndFinish to do its job
                if os.path.exists(stdout_file) and exit_code is not None:
                    with open(stdout_file, 'r') as output_file:
                        outfile = output_file.read()
                    # Modify the tester object's 'test_dir' to reflect the directory path created by QueueManager.copyFiles
                    # We need to do this because we are not copying the 'tests' file to this queued location
                    tester.specs['test_dir'] = test_info['test_dir']

                    # Allow the tester to verify its own output and set the status. This stage of the test
                    # shall not return a 'pending' status
                    output = tester.processResults(tester.specs['moose_dir'], exit_code, self.options, outfile)

                elif exit_code == None:
                    # When this was job canceled or deleted (qdel)
                    reason = 'FAILED (PBS ERROR)'
                    tester.setStatus(reason, tester.bucket_fail)
                else:
                    # I ran into this scenario when the cluster went down, but launched/completed my job :)
                    reason = 'NO STDOUT FILE'
                    tester.setStatus(reason, tester.bucket_fail)

            elif output_value.group(1) == 'R':
                # Job is currently running
                reason = 'RUNNING'
            elif output_value.group(1) == 'E':
                # Job is exiting
                reason = 'EXITING'
            elif output_value.group(1) == 'Q':
                # Job is currently queued
                reason = 'QUEUED'
            elif output_value.group(1) == 'H':
                # Job is waiting for other jobs
                reason = 'HOLDING'
            else:
                # Unknown statuses should be treated as failures
                reason = 'UNKNOWN PBS STATUS'
                tester.setStatus(reason, tester.bucket_fail)

            if reason != '' and tester.getStatus() != tester.bucket_fail:
                tester.setStatus(reason, tester.bucket_pending)
        else:
            # Job status not available
            reason = 'INVALID QSTAT RESULTS'
            tester.setStatus(reason, tester.bucket_fail)

        return output
