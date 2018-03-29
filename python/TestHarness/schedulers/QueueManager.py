#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from Scheduler import Scheduler

class QueueManager(Scheduler):
    """
    QueueManager is a Scheduler plugin responsible for allowing the testers to
    be scheduled via a third party queue system (like PBS) and to handle the
    stateful requirements associated with such a task (the session_file).
    """
    @staticmethod
    def validParams():
        params = Scheduler.validParams()
        return params

    def __init__(self, harness, params):
        Scheduler.__init__(self, harness, params)

    def _getCores(self, jobs):
        """ iterate over Jobs and collect a max core count from the group """
        slots = 0
        for job in jobs:
            if not job.isFinished():
                slots = max(slots, job.getSlots())
        return slots

    def _getWalltime(self, jobs):
        """ iterate over Jobs and increment the total walltime needed to complete the entire group """
        total_time = 0
        for job in jobs:
            if not job.isFinished():
                tester = job.getTester()
                total_time += tester.getMaxTime()
        return total_time

    def createAndLaunchJobs(self, Jobs, max_cores, walltime):
        """ derived method to create launch script and execute it """
        return 0

    def augmentJobs(self, Jobs):
        """
        Gather necessary core counts and walltimes by iterating over every job
        about to launch. Using this information, call the derived queue manager's
        runJob (like RunPBS), to create the launch script.

        We then execute this script (which should provide us with some useful
        information (like a PBS Job ID). We will append this information to
        job, along with setting each job to finish. Allowing inherited queueJobs
        to print this launch information to the user (for good or ill).
        """
        job_dag = Jobs.getDAG()
        job_list = job_dag.topological_sort()
        max_cores = self._getCores(job_list)
        walltime = self._getWalltime(job_list)

        self.createAndLaunchJobs(job_dag, max_cores, walltime)
        for job in job_list:
            tester = job.getTester()
            tester.addCaveats('LAUNCHED')
            tester.setStatus(tester.success)
            job.setStatus(job.finished)
