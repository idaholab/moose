#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from QueueManager import QueueManager
import os

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

    def createAndLaunchJobs(self, job_dag, max_cores, walltime):
        """ derived method to create launch script and execute it, returning the launch ID """
        # Get an arbitrary job. We only need to do this, to get Tester Dir
        if job_dag.size():
            job = job_dag.topological_sort()[0]
            # There is no specific single default anymore (it is tests, and speedtests)
            if not self.options.input_file_name:
                spec_file = 'tests'
            else:
                spec_file = self.options.input_file_name
            command = [os.path.join(self.harness.run_tests_dir, 'run_tests'), '--spec-file', os.path.join(job.getTestDir(), spec_file)]
            job.addMetaData(command=command, launch_id=0)
