#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from schedulers.Job import Job
from contrib import dag

class JobDAG(object):
    """ Class which builds a Job DAG for use by the Scheduler """
    def __init__(self, options):
        self.__job_dag = dag.DAG()
        self.options = options

    def createJobs(self, testers):
        """ Return a usable Job DAG based on supplied list of tester objects """
        # for each tester, instance a job and create a DAG node for that job
        self.__name_to_job = {}
        for tester in testers:
            job = Job(tester, self.__job_dag, self.options)
            name = job.getUniqueIdentifier()
            if name not in self.__name_to_job:
                self.__name_to_job[name] = job
            else:
                job.addCaveats('duplicate test')
                job.setStatus(job.skip)
            self.__job_dag.add_node(job)

        return self._checkDAG()

    def getJobs(self):
        """ return current job group """
        return self.__job_dag.ind_nodes()

    def getJobsAndAdvance(self):
        """
        return finished jobs, and remove them from the DAG, thus
        advancing to the next set of jobs when called again.
        """
        # handle any skipped dependencies
        self._doSkippedDependencies()

        # delete finished jobs
        next_jobs = set([])
        for job in list(self.__job_dag.ind_nodes()):
            if job.isFinished():
                next_jobs.add(job)
                self.__job_dag.delete_node(job)

        next_jobs.update(self.getJobs())
        return next_jobs

    def _checkDAG(self):
        """ perform some sanity checks on the current DAG """
        if self.__job_dag.size():

            self._doMakeDependencies()

            self._doSkippedDependencies()

            # If there are race conditions, then there may be more skipped jobs
            if self._doRaceConditions():
                self._doSkippedDependencies()

        return self.__job_dag

    def _doMakeDependencies(self):
        """ Setup dependencies within the current Job DAG """
        for job in self.__job_dag.ind_nodes():
            prereq_jobs = job.getUniquePrereqs()
            for prereq_job in prereq_jobs:
                try:
                    self.__name_to_job[prereq_job]
                    self.__job_dag.add_edge(self.__name_to_job[prereq_job], job)

                # Cyclic errors
                except dag.DAGValidationError:
                    err_output = self._printDownstreams(job)
                    err_output += ' %s <--> %s' % (job.getTestName().split('.')[1],
                                                 self.__name_to_job[prereq_job].getTestName().split('.')[1])

                    job.setOutput('Cyclic dependency error!\n\t' + err_output)
                    job.setStatus(job.error, 'Cyclic or Invalid Dependency Detected!')

                # test file has invalid prereq set
                except KeyError:
                    job.setStatus(job.error, 'unknown dependency')

    def _doSkippedDependencies(self):
        """ Determine which jobs in the DAG should be skipped """
        for job in list(self.__job_dag.topological_sort()):
            tester = job.getTester()
            dep_jobs = set([])
            if not job.getRunnable() or self._haltDescent(job):
                job.setStatus(job.skip)
                dep_jobs.update(self.__job_dag.all_downstreams(job))

                # Remove parent dependency so it can launch individually
                for p_job in self.__job_dag.predecessors(job):
                    self.__job_dag.delete_edge_if_exists(p_job, job)

            for d_job in dep_jobs:
                d_tester = d_job.getTester()
                if tester.isSilent() and not d_job.getRunnable():
                    d_tester.setStatus(d_tester.silent)
                elif not self._skipPrereqs():
                    d_job.setStatus(d_job.skip)
                    d_job.addCaveats('skipped dependency')

                self.__job_dag.delete_edge_if_exists(job, d_job)

    def _doRaceConditions(self):
        """ Check for race condition errors within in the DAG"""
        # Create a clone of the DAG so we can be destructive with it (we need to
        # simulate running each job to completion)
        race_conditions_found = False
        mutable_dag = self.__job_dag.clone()
        while mutable_dag.size():
            output_files_in_dir = set()
            concurrent_jobs = mutable_dag.ind_nodes()
            for job in concurrent_jobs:
                if job.isFinished():
                    continue
                tester = job.getTester()
                output_files = tester.getOutputFiles()
                if len(output_files_in_dir.intersection(set(output_files))):
                    race_conditions_found = True
                    # Break out of this loop and set every job involved as a failure
                    # for race conditions
                    for this_job in concurrent_jobs:
                        this_job.setStatus(this_job.error, 'OUTFILE RACE CONDITION')
                    break
                output_files_in_dir.update(output_files)
            for job in concurrent_jobs:
                mutable_dag.delete_node(job)

    def _haltDescent(self, job):
        """ return boolean if this job should not allow its children to run """
        tester = job.getTester()
        if (job.isFail()
            or job.isSkip()
            or tester.isFail()
            or tester.isSkip()
            or tester.isSilent()
            or tester.isDeleted()):
            return True

    def _skipPrereqs(self):
        """
        Method to return boolean to skip dependency prerequisites checks.
        """
        if (self.options.ignored_caveats
            and ('all' in self.options.ignored_caveats
                 or 'prereq' in self.options.ignored_caveats)):
            return True

    def _printDownstreams(self, job):
        """
        create a printable dependency chart of for supplied job
        # TODO: It would be super cool to print the entire DAG
                in this fashion.
        """
        downstreams = self.__job_dag.all_downstreams(job)
        cyclic_path = []
        for d_job in downstreams:
            cyclic_path.append('%s -->'% (d_job.getTestNameShort()))
        return ' '.join(cyclic_path)
