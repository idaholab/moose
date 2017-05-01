from timeit import default_timer as clock
from MooseObject import MooseObject
from QueueManager import QueueManager
import os, re, json
## This class launches jobs using the PBS queuing system
class RunPBS(QueueManager):
    @staticmethod
    def validParams():
        params = QueueManager.validParams()
        params.addRequiredParam('scheduler',       'RunPBS',       "the name of the scheduler used")
        params.addRequiredParam('place',           'free',         "node placement")
        params.addRequiredParam('walltime',        '',             "amount of time to request for this test")
        params.addRequiredParam('template_script', 'template',     "the template script used")

        params.addParam('combine_streams',  '#PBS -j oe',           "combine stdout and stderr")
        params.addParam('pbs_queue',                  '',           "the PBS queue to use")
        params.addParam('pbs_project',                '',           "the PBS project to use")
        params.addParam('pbs_prereq',                 '',           "list of pbs jobs this job depends on")


        return params

    ## Return this return code if the process must be killed because of timeout
    TIMEOUT = -999999

    def __init__(self, harness, params):
        QueueManager.__init__(self, harness, params)
        self.specs = params

    # write options to qsub batch file
    def prepareLaunch(self, tester):
        return

    def getQueueCommand(self, tester):
        if self.options.processingQueue:
            return 'qstat -xf %s' % (self.queue_data[tester.specs['test_name']]['id'])
        else:
            return 'qsub %s' % (self.queue_script)

    # Read the template file make some changes, and write the launch script
    def prepareQueueScript(self, preq_list):
        # Add prereq job id, now that we know them!
        if len(preq_list):
            self.specs['pbs_prereq'] = '#PBS -W depend=afterany:%s' % (':'.join(preq_list))

        f = open(self.specs['template_script'], 'r')
        content = f.read()
        f.close()

        params = self.specs

        f = open(self.queue_script, 'w')

        # Do all of the replacements for the valid parameters
        for param in params.valid_keys():
            if param in params.substitute:
                params[param] = params.substitute[param].replace(param.upper(), params[param])
            content = content.replace('<' + param.upper() + '>', str(params[param]))

        # Make sure we strip out any string substitution parameters that were not supplied
        for param in params.substitute_keys():
            if not params.isValid(param):
                content = content.replace('<' + param.upper() + '>', '')

        f.write(content)
        f.close()

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
    def handleQsubOutput(self, tester, output):
        queue_job_id = 0
        if 'command not found' in output:
            tester.setStatus('QSUB NOT FOUND', tester.bucket_fail)
        else:
            test_output = os.path.join(self.specs['working_dir'], self.specs['job_name'])
            job_id = re.search(r'^(\d+)\.[\W\w]+$', output)

            # Job was launched and we have a PBS Job ID
            if job_id:
                queue_job_id = job_id.group(1)
                tester.setStatus('%s LAUNCHED' % (str(queue_job_id)), tester.bucket_pending)

            # Something went wrong trying to launch the job
            else:
                tester.setStatus('QSUB INVALID RESULTS: %s' % (output), tester.bucket_fail)

        # Update the queue_data global so that it can be saved/retreived to/from the json file.
        self.queue_data[tester.specs['test_name']] = { 'test_name'     : tester.specs['test_name'],
                                                       'input_name'    : self.options.input_file_name,
                                                       'std_out'       : test_output + '.o%s' % (queue_job_id),
                                                       'std_err'       : test_output + '.e%s' % (queue_job_id),
                                                       'test_dir'      : self.specs['working_dir'],
                                                       'status_text'   : tester.getStatusMessage(),
                                                       'id'            : queue_job_id }
        # Write the json data to queue file
        json.dump(self.queue_data, self.queue_file, indent=2)

        return output

    # Handle statuses supplied by qstat output
    def handleQstatOutput(self, tester, qstat_output):
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
                test_info = self.queue_data[tester.specs['test_name']]

                # Read the stdout file and allow testOutputAndFinish to do its job
                if os.path.exists(test_info['std_out']) and exit_code is not None:
                    with open(test_info['std_out'], 'r') as output_file:
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

    ## Return control to the test harness by finalizing the test output and calling the callback
    ## TODO: refactor much of this into the Scheduler class
    def returnToTestHarness(self, job_index):
        (p, command, tester, time, f, slots) = self.jobs[job_index]

        log( 'Command %d done:    %s' % (job_index, command) )
        output = 'Working Directory: ' + tester.specs['test_dir'] + '\nRunning command: ' + command + '\n'

        if p.poll() == None: # process has not completed, it timed out
            output += '\n' + "#"*80 + '\nProcess terminated by test harness. Max time exceeded (' + str(tester.specs['max_time']) + ' seconds)\n' + "#"*80 + '\n'
            f.close()
            tester.setStatus('TIMEOUT', tester.bucket_fail)
            if platform.system() == "Windows":
                p.terminate()
            else:
                pgid = os.getpgid(p.pid)
                os.killpg(pgid, SIGTERM)

            self.harness.testOutputAndFinish(tester, RunPBS.TIMEOUT, output, time, clock())
        else:

            # Get stdout from qstat/qsub
            queue_output = self.readOutput(f)
            f.close()

            # determine tester statuses depending on mode (qsub | qstat)
            if self.options.processingQueue:
                # QSTAT method because the test was already launched
                output += self.handleQstatOutput(tester, queue_output)
            else:
                # QSUB method because the test needed to be launched
                output += self.handleQsubOutput(tester, queue_output)

            if tester.getStatus() != tester.bucket_pending:
                self.finished_jobs.add(tester.specs['test_name'])
                self.harness.testOutputAndFinish(tester, p.returncode, output, time, clock())
            else:
                self.skipped_jobs.add(tester.specs['test_name'])
                self.harness.handleTestStatus(tester, output)

        self.jobs[job_index] = None
        self.slots_in_use = self.slots_in_use - slots

## Static logging string for debugging
LOG = []
LOG_ON = False
def log(msg):
    if LOG_ON:
        LOG.append(msg)
        print msg
