#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, re
from QueueManager import QueueManager
from TestHarness import util # to execute qsub

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
        self.harness = harness
        self.options = self.harness.getOptions()

    def getBadKeyArgs(self):
        """ arguments we need to remove from sys.argv """
        return ['--pbs']

    def hasTimedOutOrFailed(self, job_data):
        """ use qstat and return bool on job failures outside of the TestHarness's control """
        launch_id = job_data.json_data.get(job_data.job_dir,
                                           {}).get(job_data.plugin,
                                                   {}).get('ID', "").split('.')[0]

        # We shouldn't run into a null, but just in case, lets handle it
        if launch_id:
            qstat_command_result = util.runCommand('qstat -xf %s' % (launch_id))

            # handle a qstat execution failure for some reason
            if qstat_command_result.find('ERROR') != -1:
                # set error for each job contained in group
                for job in job_data.jobs.getJobs():
                    job.setOutput('ERROR invoking `qstat`\n%s' % (qstat_command_result))
                    job.setStatus(job.error, 'QSTAT')
                return True

            qstat_job_result = re.findall(r'Exit_status = (\d+)', qstat_command_result)

            # woops. This job was killed by PBS by exceeding walltime
            if qstat_job_result and qstat_job_result[0] == "271":
                for job in job_data.jobs.getJobs():
                    job.addCaveats('Killed by PBS Exceeded Walltime')
                return True

            # Capture TestHarness exceptions
            elif qstat_job_result and qstat_job_result[0] != "0":

                # Try and gather some useful output we can tack on to one of the job objects
                output_file = job_data.json_data.get(job_data.job_dir, {}).get(job_data.plugin, {}).get('QSUB_OUTPUT', "")
                if os.path.exists(output_file):
                    with open(output_file, 'r') as f:
                        output_string = util.readOutput(f, None, self.options)
                    job_data.jobs.getJobs()[0].setOutput(output_string)

                # Add a caveat to each job, explaining that one of the jobs caused a TestHarness exception
                for job in job_data.jobs.getJobs():
                    job.addCaveats('TESTHARNESS EXCEPTION')
                return True

    def _augmentTemplate(self, job):
        """ populate qsub script template with paramaters """
        template = {}

        # Launch script location
        template['launch_script'] = os.path.join(job.getTestDir(), job.getTestNameShort() + '.qsub')

        # NCPUS
        template['mpi_procs'] = job.getMetaData().get('QUEUEING_NCPUS', 1)

        # Convert MAX_TIME to hours:minutes for walltime use
        max_time = job.getMetaData().get('QUEUEING_MAXTIME', 1)
        hours = int(int(max_time) / 3600)
        minutes = int(int(max_time) / 60) % 60
        template['walltime'] = '{0:02d}'.format(hours) + ':' + '{0:02d}'.format(minutes) + ':00'

        # Job Name
        template['job_name'] = job.getTestNameShort()

        # PBS Project group
        template['pbs_project'] = '#PBS -P %s' % (self.options.queue_project)

        # PBS Queue
        if self.options.queue_queue:
            template['pbs_queue'] = '#PBS -q %s' % (self.options.queue_queue)
        else:
            template['pbs_queue'] = ''

        # Apply source command
        if self.options.queue_source_command and os.path.exists(self.options.queue_source_command):
            template['pre_command'] = 'source %s || exit 1' % (os.path.abspath(self.options.queue_source_command))
        else:
            template['pre_command'] = ''

        # Redirect stdout to this location
        template['output'] = os.path.join(job.getTestDir(), 'qsub.output')

        # Root directory
        template['working_dir'] = self.harness.base_dir

        # Command
        template['command'] = ' '.join(self.getRunTestsCommand(job))

        return template

    def run(self, job):
        """ execute qsub and return the launch id """
        template = self._augmentTemplate(job)
        tester = job.getTester()

        self.createQueueScript(job, template)

        command = ' '.join(['qsub', template['launch_script']])
        launch_results = util.runCommand(command, job.getTestDir())

        # List of files we need to clean up when we are done
        dirty_files = [template['launch_script'],
                       template['output']]

        self.addDirtyFiles(job, dirty_files)

        if launch_results.find('ERROR') != -1:
            # The executor job failed (so fail all jobs in this group)
            job_dag = job.getDAG()

            for other_job in [x for x in job_dag.topological_sort() if x != job]:
                other_job.clearCaveats()
                other_tester = other_job.getTester()
                other_tester.setStatus(other_tester.fail, 'launch failure')

            # This is _only_ to make the failed message more useful
            tester.specs['test_dir'] = ''
            tester.specs['command'] = command
            tester.setStatus(tester.fail, 'QSUB Group Failure')
            job.setOutput(launch_results)

        else:
            job.addMetaData(RunPBS={'ID' : launch_results,
                                    'QSUB_COMMAND' : command,
                                    'NCPUS' : template['mpi_procs'],
                                    'WALLTIME' : template['walltime'],
                                    'QSUB_OUTPUT' : template['output']})
            tester.setStatus(tester.no_status, 'LAUNCHING')
