#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from .schedulers.Job import Job
from contrib import dag
import sys
import threading

class JobDAG(object):
    """ Class which builds a Job DAG for use by the Scheduler """
    def __init__(self, options, parallel_scheduling):
        self.options = options
        self.__parallel_scheduling = parallel_scheduling
        self.__job_dag = dag.DAG()
        self.__j_lock = threading.Lock()

    def getLock(self):
        """ Return the lock for this test spec (folder of jobs) """
        return self.__j_lock

    def canParallel(self):
        """ Return bool whether or not this group runs in parallel """
        return self.__parallel_scheduling

    def createJobs(self, testers):
        """ Return a usable Job DAG based on supplied list of tester objects """
        # for each tester, instance a job and create a DAG node for that job
        self.__name_to_job = {}
        for tester in testers:
            job = Job(tester, self, self.options)
            name = job.getTestNameShort()
            if name not in self.__name_to_job:
                self.__name_to_job[name] = job
            else:
                job.addCaveats('duplicate test')
                job.setStatus(job.skip)
            self.__job_dag.add_node(job)

        return self._checkDAG()

    def getDAG(self):
        """ return the running DAG object """
        return self.__job_dag

    def getAvailableJobs(self):
        """ Return a list of available jobs """
        available_jobs = [job for job in self.__job_dag.ind_nodes() if job.isHold()]
        if self.canParallel() and not self.options.pedantic_checks:
            return available_jobs
        return available_jobs[0:1]

    def getJobsAndAdvance(self):
        """
        return finished jobs, and remove them from the DAG, thus
        advancing to the next set of jobs when called again.
        """
        # handle any skipped dependencies
        self._doSkippedDependencies()
        next_jobs = set([])
        for job in list(self.__job_dag.ind_nodes()):
            if job.isFinished():
                next_jobs.add(job)
                self.__job_dag.delete_node(job)

        next_jobs.update(self.getAvailableJobs())
        return next_jobs

    def removeAllDependencies(self):
        """ Flatten current DAG so that it no longer contains any dependency information """
        if self.__name_to_job and self.__job_dag.size():
            tmp_job_dag = dag.DAG()
            for job in self.getJobs():
                tmp_job_dag.add_node(job)
            self.__job_dag = tmp_job_dag
        return self.__job_dag

    def _checkDAG(self):
        """ perform some sanity checks on the current DAG """
        if self.__job_dag.size():
            # Add edges based on prereqs
            self._setupPrereqs()

            # Check for race conditions in output
            self._checkOutputCollisions()

            # Remove edges for jobs that are skipped
            self._doSkippedDependencies()

        return self.__job_dag

    def _addEdge(self, child, parent):
        try:
            self.__job_dag.add_edge(child, parent)
        # Cyclic errors
        except dag.DAGValidationError:
            err_output = self._printDownstreams(parent)
            err_output += ' %s <--> %s' % (parent.getTestName().split('.')[1],
                                           child.getTestName().split('.')[1])

            parent.appendOutput('Cyclic dependency error!\n\t' + err_output)
            parent.setStatus(parent.error, 'Cyclic or Invalid Dependency Detected!')

    def _setupPrereqs(self):
        """ Setup dependencies within the current Job DAG """
        # The jobs that have 'ALL' as a prereq
        all_prereq_jobs = []

        # Setup explicit dependencies (without 'ALL')
        for job in self.__job_dag.ind_nodes():
            prereq_jobs = job.getPrereqs()
            if prereq_jobs == ['ALL']:
                all_prereq_jobs.append(job)
                continue
            for prereq_job in prereq_jobs:
                try:
                    self.__name_to_job[prereq_job]
                    self._addEdge(self.__name_to_job[prereq_job], job)
                    # Fix heavy test corner cases
                    self._fix_cornercases(self.__name_to_job[prereq_job], job)

                # test file has invalid prereq set
                except KeyError:
                    job.setStatus(job.error, f'unknown dependency {prereq_job}')

        # Setup dependencies for 'ALL'
        for job in all_prereq_jobs:
            for a_job in self.getJobs():
                if a_job != job and not a_job.isSkip():
                    if '.ALL' in a_job.getTestName():
                        a_job.setStatus(a_job.error, 'Test named ALL when "prereq = ALL" elsewhere in test spec file!')
                    self._addEdge(a_job, job)

    def _fix_cornercases(self, prereq_job, job):
        """
        Fix skipped dependency when we have a heavy test depend on a not-heavy test
        TODO: We discovered several other cases where tests may never run. However,
              solving those issues is a rabbit hole better left to another PR: #26329
        """
        if self.options.heavy_tests and job.specs['heavy']:
            prereq_tester = prereq_job.getTester()
            if not prereq_tester.specs['heavy']:
                prereq_tester.specs['heavy'] = True
                prereq_tester.addCaveats(f'implicit heavy')

    def _hasDownStreamsWithFailures(self, job):
        """ Return True if any dependents of job has previous failures """
        for d_job in self.__job_dag.all_downstreams(job):
            status, message, caveats = d_job.previousTesterStatus()
            if status in d_job.job_status.getFailingStatuses():
                return True

    def _doPreviouslyFailed(self, job):
        """
        Set up statuses for jobs contained within the DAG for use with failed-tests option
        """
        tester = job.getTester()
        status, message, caveats = job.previousTesterStatus()

        # This job passed, but one of its dependents has not
        if status == tester.success and self._hasDownStreamsWithFailures(job):
            tester.addCaveats('re-running')
            return

        # This job was skipped, passed, pending or silent
        elif status in job.job_status.getSuccessStatuses() or status in job.job_status.getPendingStatuses():
            tester.setStatus(tester.silent)
            job.setStatus(job.finished)

        # Remaining independent 'skipped' jobs we don't want to print output for
        elif not job.getRunnable():
            tester.setStatus(tester.silent)
            job.setStatus(job.finished)

        # Remaining jobs are failures of some sort. Append the previous result as a caveat.
        if message:
            tester.addCaveats('previous results: {}'.format(message))

    def _doSkippedDependencies(self):
        """ Determine which jobs in the DAG should be skipped """
        for job in self.getJobs():
            dep_jobs = set([])

            if self.options.failed_tests:
                self._doPreviouslyFailed(job)

            if not job.getRunnable() or job.isFail() or job.isSkip():
                job.setStatus(job.skip)
                dep_jobs.update(self.__job_dag.all_downstreams(job))

                # Remove parent dependency so it can launch individually
                for p_job in self.__job_dag.predecessors(job):
                    self.__job_dag.delete_edge_if_exists(p_job, job)

            for d_job in dep_jobs:
                d_tester = d_job.getTester()
                if job.isSilent() and not d_job.getRunnable():
                    d_tester.setStatus(d_tester.silent)
                elif not self._skipPrereqs():
                    d_job.setStatus(d_job.skip)
                    d_job.addCaveats('skipped dependency')
                self.__job_dag.delete_edge_if_exists(job, d_job)

    def _checkOutputCollisions(self):
        """
        If running in parallel, checks to see if any tests have outputs
        that would collide when ran in parallel if prereqs are set.
        """
        # No need to check if this spec can't run in parallel, because
        # all tests will be run sequentially, with no more than one at once
        if not self.canParallel():
            return

        # Sort by ID so we get it in the input file from top down
        jobs = sorted(self.getJobs(), key = lambda job: job.getID())

        # Work down the file, starting with the second input and looking up for
        # collisions. By doing it in this order, we will error at the first occurance.
        # This is nice because if we list all of the collisions it could be a lot of
        # confusing output
        for i in range(1, len(jobs)):
            job = jobs[i]
            for other_i in reversed(range(i)):
                other_job = jobs[other_i]
                tester = job.getTester()
                other_tester = other_job.getTester()
                files = set(tester.getOutputFiles(self.options))
                other_files = set(other_tester.getOutputFiles(self.options))
                conflicting_files = list(files.intersection(other_files))
                if conflicting_files \
                    and not self.__job_dag.is_dependency(other_job, job) \
                    and not self.__job_dag.is_dependency(job, other_job):
                    print('  This test spec is set to run in parallel, but a race condition was found')
                    print('  that could lead to multiple tests reading/writing from the same file.\n')
                    print(f'  Tests: {tester.getTestNameShort()}, {other_tester.getTestNameShort()}')
                    print(f'  File(s): {", ".join(conflicting_files)}\n')
                    print('  You can resolve this issue by setting the approprate prerequisites')
                    print('  between your tests with the "prereq" parameter')
                    sys.exit(1)

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

    def getJobs(self):
        """
        Returns the sorted jobs in the DAG
        """
        return self.__job_dag.topological_sort()

    def printDAG(self):
        """ Print the current structure of the DAG  """
        job_order = []
        cloned_dag = self.__job_dag.clone()
        while cloned_dag.size():
            concurrent_jobs = cloned_dag.ind_nodes(cloned_dag.graph)
            if len(concurrent_jobs) > 1:
                job_order.extend([x.getTestNameShort() for x in concurrent_jobs])
            else:
                if job_order:
                    job_order.extend(['<--', concurrent_jobs[0].getTestNameShort()])
                else:
                    job_order.append(concurrent_jobs[0].getTestNameShort())
            for job in concurrent_jobs:
                cloned_dag.delete_node(job)

        print('\n###### JOB ORDER ######\n', ' '.join(job_order))
