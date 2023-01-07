#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, sys, re, json
from QueueManager import QueueManager
from TestHarness import util # to execute qsub
import math # to compute node requirement
from PBScodes import *

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

    def _readJobOutput(self, output_file, N=5):
        """ return last few lines in output_file for job group """
        output = []
        if os.path.exists(output_file):
            with open(output_file, 'r') as outfile:
                for line in (outfile.readlines() [-N:]):
                    output.append(line)
            output.append(f'Last {N} lines read. Full output file available at:\n{output_file}')
        return '\n'.join(output)

    def hasQueuingFailed(self, job_data):
        """ Determine if PBS killed the job prematurely """
        queue_plugin = self.__class__.__name__
        jobs = job_data.jobs.getJobs()
        meta_data = job_data.json_data.get(jobs[0].getTestDir())
        launch_id = meta_data.get(queue_plugin, {}).get('ID', '').split('.')[0]
        output_file = os.path.join(jobs[0].getTestDir(), 'qsub.output')

        # Job was never originally launched
        if not meta_data.get(queue_plugin, False) or not launch_id:
            return False

        # Job ran to completion
        elif os.path.exists(os.path.join(jobs[0].getTestDir(), '.previous_test_results.json')):
            return False

        ### Job has some other status ###

        # Check qstat for current status
        qstat_command_result = util.runCommand(f'qstat -xf -F json {launch_id}')

        # Catch json parsing errors
        try:
            json_out = json.loads(qstat_command_result)
            pbs_server = json_out['pbs_server']
            job_meta = json_out['Jobs'][f'{launch_id}.{pbs_server}']

        # JobID no longer exists (stale after 1 week)
        except json.decoder.JSONDecodeError:
            # Job did not run to completion (no .previous_test_results.json file exists)
            if os.path.exists(output_file):
                qstat_command_result = (f'ERROR: {self._readJobOutput(output_file)}'
                                        '\n\nMore information available in\n'
                                        f' {output_file}\n')

            # Failed parse, and no output file. Perhaps the PBS job was canceled, deleted, etc
            else:
                qstat_command_result = ('ERROR: TestHarness encountered an error while'
                                       f'determining what to make of PBS JobID {launch_id}:\n'
                                       f'{qstat_command_result}')

        # Handle a qstat execution failure
        if qstat_command_result.find('ERROR') != -1:
            for job in job_data.jobs.getJobs():
                job.setOutput(f'ERROR invoking `qstat`:\n{qstat_command_result}')
                job.setStatus(job.error, 'QSTAT')
            return True

        # Use qstat json output to examine current status
        job_result = job_meta.get('Exit_status', False)

        # Set the status as seen by qstat
        meta_data[self.__class__.__name__]['STATUS'] = PBS_STATUSES[job_meta['job_state']]

        # Woops. This job was killed by PBS for some reason
        if job_result and str(job_result) in PBS_User_EXITCODES.keys():
            output = f'{self._readJobOutput(output_file)}\n{PBS_User_EXITCODES[str(job_result)]}'
            for job in jobs:
                job.setOutput(output)
                job.addCaveats(f'PBS ERROR: {job_result}')
            return True

        # Capture TestHarness exceptions
        elif job_result and job_result != "0":
            if os.path.exists(output_file):
                with open(output_file, 'r') as f:
                    output_string = util.readOutput(f, None, jobs[0].getTester())
                jobs[0].setOutput(output_string)
            # Add a caveat to each job, explaining that one of the jobs caused a TestHarness exception
            for job in jobs:
                job.addCaveats('TESTHARNESS EXCEPTION')
            return True

        return False

    def _augmentTemplate(self, job):
        """ populate qsub script template with paramaters """
        job_data = self.options.results_storage.get(job.getTestDir(), {})
        queue_meta = job_data.get(self.__class__.__name__, { self.__class__.__name__: {} })

        template = {}

        # Launch script location
        template['launch_script'] = os.path.join(job.getTestDir(), os.path.basename(job.getTestNameShort()) + '.qsub')

        # NCPUS
        template['mpi_procs'] = queue_meta.get('QUEUEING_NCPUS', 1)

        # Compute node requirement
        if self.options.pbs_node_cpus and template['mpi_procs'] > self.options.pbs_node_cpus:
            nodes = template['mpi_procs']/self.options.pbs_node_cpus
            template['mpi_procs'] = self.options.pbs_node_cpus
        else:
            nodes = 1
        template['nodes'] = math.ceil(nodes)

        # Convert MAX_TIME to hours:minutes for walltime use
        max_time = queue_meta.get('QUEUEING_MAXTIME', 1)
        hours = int(int(max_time) / 3600)
        minutes = int(int(max_time) / 60) % 60
        template['walltime'] = '{0:02d}'.format(hours) + ':' + '{0:02d}'.format(minutes) + ':00'

        # Job Name
        template['job_name'] = os.path.basename(job.getTestNameShort())

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
        template['command'] = ' '.join(self.getRunTestsCommand(job, template['mpi_procs']))

        return template

    def run(self, job):
        """ execute qsub and return the launch id """
        tester = job.getTester()
        if self.options.dry_run:
            tester.setStatus(tester.success, 'DRY_RUN')
            return

        template = self._augmentTemplate(job)
        job_meta = self.options.results_storage.get(job.getTestDir(), { job.getTestDir() : {} })
        self.createQueueScript(job, template)
        command = ' '.join(['qsub', template['launch_script']])
        launch_results = util.runCommand(command, job.getTestDir())

        # List of files we need to clean up when we are done
        dirty_files = [template['launch_script'],
                       template['output'],
                       os.path.join(job.getTestDir(), self.harness.results_file)]

        if launch_results.find('ERROR') != -1:
            # The executor job failed (so fail all jobs in this group)
            job_dag = job.getDAG()

            for other_job in [x for x in job_dag.topological_sort() if x != job]:
                other_job.clearCaveats()
                other_tester = other_job.getTester()
                other_tester.setStatus(other_tester.fail, 'launch failure')

            # This is _only_ to make the failed message more useful
            tester.specs['command'] = command
            tester.setStatus(tester.fail, 'QSUB Group Failure')
            job.setOutput(launch_results)

        else:
            # While RunPBS believes this was a successful launch, perhaps this system's PBS system
            # failed to launch for some other strange reason, and didn't error (above .find(ERROR)
            # routine). In which case, it helps to set some 'past tense' grammar as our result
            # in our '--pbs some_name' json file
            job_meta[self.__class__.__name__].update({'ID'           : launch_results,
                                                      'QSUB_COMMAND' : command,
                                                      'NCPUS'        : template['mpi_procs'],
                                                      'WALLTIME'     : template['walltime'],
                                                      'QSUB_OUTPUT'  : template['output'],
                                                      'STATUS'       : 'PREVIOUSLY LAUNCHED',
                                                      'DIRTY_FILES'  : dirty_files})

            tester.setStatus(tester.queued, 'LAUNCHING')
