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

    def getDAG(self):
        """ return the running DAG object """
        return self.__job_dag


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

    def removeAllDependencies(self):
        """ Flatten current DAG so that it no longer contains any dependency information """
        if self.__name_to_job and self.__job_dag.size():
            tmp_job_dag = dag.DAG()
            for job in self.__job_dag.topological_sort():
                tmp_job_dag.add_node(job)
            self.__job_dag = tmp_job_dag
        return self.__job_dag

    def _checkDAG(self):
        """ perform some sanity checks on the current DAG """
        if self.__job_dag.size():

            self._doMakeDependencies()

            # If checking for race conditions, run do the following.
            if self.options.testharness_diagnostics:
                # If we run this, we need a copy of the DAG before the extra edges are added
                for name in self.__name_to_job:
                    temp_downstream_job_list = self.__job_dag.all_downstreams(self.__name_to_job[name])
                    for job in temp_downstream_job_list:
                        self.__name_to_job[name].addDownsteamNode(job)
                    temp_upstream_job_list = self.__job_dag.predecessors(self.__name_to_job[name])
                    for job in temp_upstream_job_list:
                        self.__name_to_job[name].addUpsteamNode(job)

                self._doMakeSerializeDependencies()

            self._doSkippedDependencies()

            # If there are race conditions, then there may be more skipped jobs
            if self._doRaceConditions():
                self._doSkippedDependencies()

        return self.__job_dag

    def _doMakeSerializeDependencies(self):
        return self.__job_dag.serialize_dag()

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
                if not self.options.testharness_diagnostics:
                    dep_jobs.update(self.__job_dag.all_downstreams(job))
                # If running --diag, we need to use the original downstreams, rather
                # than the downstreams in the current DAG
                else:
                    dep_jobs.update(job.getDownstreamNodes())

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

                if not self.options.testharness_diagnostics:
                    self.__job_dag.delete_edge_if_exists(job, d_job)
                else:
                    try:
                        job.removeDownsteamNode(d_job)
                    except:
                        pass

    def _doRaceConditions(self):
        """ Check for race condition errors within in the DAG"""
        # Build output_file in relation to job dictionary
        output_to_job = {}
        for job in self.__job_dag.topological_sort():
            if job.getRunnable() and not job.isFinished():
                for output_file in job.getOutputFiles():
                    output_to_job[output_file] = output_to_job.get(output_file, [])
                    output_to_job[output_file].append(job)

        # Remove jobs which have accurate dependencies
        for outfile, job_list in output_to_job.iteritems():
            for job in list(job_list):
                for match_job in self.__job_dag.all_downstreams(job):
                    if match_job in job_list:
                        job_list.remove(match_job)

        # Left over multiple items in job_list are problematic
        for outfile, job_list in output_to_job.iteritems():
            # Same test has duplicate output files
            if len(job_list) > 1 and len(set(job_list)) == 1:
                job_list[0].setOutput('Duplicate output files:\n\t%s\n' % (outfile))
                job_list[0].setStatus(job.error, 'DUPLICATE OUTFILES')

            # Multiple tests will clobber eachothers output file
            elif len(job_list) > 1:
                for job in job_list:
                    job.setOutput('Output file will over write pre-existing output file:\n\t%s\n' % (outfile))
                    job.setStatus(job.error, 'OUTFILE RACE CONDITION')

    def _haltDescent(self, job):
        """ return boolean if this job should not allow its children to run """
        tester = job.getTester()
        if (job.isError()
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
