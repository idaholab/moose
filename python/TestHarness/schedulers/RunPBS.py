from timeit import default_timer as clock
from MooseObject import MooseObject
from QueueManager import QueueManager
import os, re, subprocess
from util import *

## This Class is responsible for maintaining an interface to the PBS scheduling syntax
class RunPBS(QueueManager):
    @staticmethod
    def validParams():
        params = QueueManager.validParams()
        params.addRequiredParam('scheduler', 'RunPBS',       "the name of the scheduler used")
        params.addRequiredParam('place',       'free',       "node placement")

        params.addParam('combine_streams',  '#PBS -j oe',           "combine stdout and stderr")
        params.addParam('pbs_queue',                  '',           "the PBS queue to use")
        params.addParam('pbs_project',                '',           "the PBS project to use")

        return params

    def __init__(self, harness, params):
        QueueManager.__init__(self, harness, params)

        # store job ids launched to qrls later
        self.launched_ids = []

    # Return command to release all launched jobs
    def getpostQueueCommand(self):
        if not self.options.checkStatus:
            if len(self.launched_ids):
                print 'Releasing jobs...'
                return 'qrls ' + ' '.join(self.launched_ids)

    # Return command necessary for current mode
    def getQueueCommand(self, tester):
        if self.options.checkStatus:
            job = self.getData(self.getJobName(tester), id=True)
            return 'qstat -xf %s' % (job['id'])
        else:
            return 'qsub -h %s' % (self.getQueueScript(tester))

    # Read the template file make some changes, and write the launch script
    def prepareQueueScript(self, template_queue, tester, preq_list):
        # Add prereq job id, now that we know them!
        template_queue['prereq'] = ''
        if len(preq_list):
            template_queue['prereq'] = '#PBS -W depend=afterany:%s' % (':'.join(preq_list))
        return template_queue

    # Augment PBS Queue specs for tester params
    def updateParams(self, template_queue, tester):
        # Set place
        template_queue['place'] = self.specs['place']

        # Use the PBS template
        template_queue['template_script'] = os.path.join(tester.specs['moose_dir'], 'python/TestHarness/schedulers', 'pbs_template')

        # Convert MAX_TIME to hours:minutes for walltime use
        hours = int(int(tester.specs['max_time']) / 3600)
        minutes = int(int(tester.specs['max_time']) / 60) % 60
        template_queue['walltime'] = '{0:02d}'.format(hours) + ':' + '{0:02d}'.format(minutes) + ':00'

        # Add PBS project directive
        if self.options.project:
            template_queue['pbs_project'] = '#PBS -P %s' % (self.options.project)

        # Add PBS queue directive
        if self.options.queue:
            template_queue['pbs_queue'] = '#PBS -q %s' % (self.options.queue)

        return template_queue

    # Handle statuses supplied by qsub output
    def handleQueueLaunch(self, tester, output):
        pattern = re.compile(r'^(\d+)\.[\W\w]+$')

        if pattern.search(output):
            job_id = pattern.search(output).group(1)

            # Update the queue_data based on results
            job = self.getData(self.getJobName(tester), job_name=True, test_dir=True)
            self.putData(self.getJobName(tester), id=job_id, std_out=os.path.join(job['test_dir'], job['job_name'] + '.o%s' %(job_id)))

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

            # If we have an exit code from PBS, go ahead and update the queue data
            self.putData(self.getJobName(tester), exit_code=exit_code)

        # Set the initial reason for a test NOT to be finished
        reason = ''

        # For all results excect Finished, return actual qstat output (useful for --verbose output)
        output = qstat_output

        if output_value:
            # Get job information from queue_data
            job = self.getData(self.getJobName(tester), std_out=True, test_dir=True)

            # Save oringal test_dir so we can properly update the queue_data
            original_testdir = tester.specs['test_dir']

            ## Report the current status of JOB_ID
            if output_value.group(1) == 'F':

                # Modify the tester object's 'test_dir' to reflect the directory path created by QueueManager.copyFiles
                # We need to do this because tester.processResults will be incorrectly operating 'up one' directory
                # from our current queued working directory.
                tester.specs['test_dir'] = job['test_dir']

                # Location of our stdout file generated by running third party queue commands
                stdout_file = job['std_out']

                # Read the stdout file and allow testOutputAndFinish to do its job
                if os.path.exists(stdout_file) and exit_code is not None:
                    with open(stdout_file, 'r') as output_file:
                        outfile = output_file.read()

                    if tester.hasRedirectedOutput(self.options):
                        outfile += self.getOutputFromFiles(tester)

                    # Allow the tester to verify its own output and set the status
                    output = tester.processResults(tester.specs['moose_dir'], exit_code, self.options, outfile)

                elif exit_code == None:
                    # This job was canceled or deleted (qdel)
                    reason = 'FAILED (PBS ERROR)'
                    tester.setStatus(reason, tester.bucket_fail)
                else:
                    # The job is done but there is no stdout file to read. Can happen when PBS had issues of some kind.
                    reason = 'NO STDOUT FILE'
                    tester.setStatus(reason, tester.bucket_fail)

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

            # Update the reason why this test is still pending
            if tester.isPending():
                tester.setStatus(reason, tester.bucket_pending)

            # Because we may have modified it earlier
            tester.specs['test_dir'] = original_testdir
        else:
            # Job status not available
            reason = 'INVALID QSTAT RESULTS'
            tester.setStatus(reason, tester.bucket_fail)

        return output
