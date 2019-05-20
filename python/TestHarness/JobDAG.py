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
#import os
#import UsingStuff
#import writingFiles


#print("JobDAG Enter.\n")
class JobDAG(object):
    """ Class which builds a Job DAG for use by the Scheduler """
    def __init__(self, options):
#        print("__init__\n")
        self.__job_dag = dag.DAG()
        self.options = options
        self.sorted = False
        self.__preSerializationDAG = dag.DAG()
        # self.ortimes = dict()
        # self.netimes = dict()
        #self.__job_dag.credgtop(self.__job_dag.topological_sort())


    def createJobs(self, testers):
        """ Return a usable Job DAG based on supplied list of tester objects """
        # for each tester, instance a job and create a DAG node for that job
#        print("createJobs\n\n")
#        print(self.options)
#        print('\n')
        self.__name_to_job = {}
        for tester in testers:
#            print(tester.getTestName())
            job = Job(tester, self.__job_dag, self.options)
            name = job.getUniqueIdentifier()
#            print(name)
#            print("\n\n\n")
#            print(self.__name_to_job)
#            print("\n\n\n")
            if name not in self.__name_to_job:
                self.__name_to_job[name] = job
            else:
                job.addCaveats('duplicate test')
                job.setStatus(job.skip)
            self.__job_dag.add_node(job)

        return self._checkDAG()



##############################################################################


    def getDAG(self):
        """ return the running DAG object """
#        print("getDAG\n")
        return self.__job_dag

    #!!!!!!!!!!!!!!!! Pending Removal !!!!!!!!!!!!!!!!#
    def getPreSerializationDAG(self):
        """ Return the preSerializedDAG associated with this tester """
        return self.__preSerializationDAG
    #!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!#

    def getJobs(self):
        """ return current job group """
#        if not self.sorted:
#            self.__job_dag.credgtop(self.__job_dag.topological_sort())
#            self.sorted = 1
        #print("getJobs\n")
        return self.__job_dag.ind_nodes()

    def getJobsAndAdvance(self):
        """
        return finished jobs, and remove them from the DAG, thus
        advancing to the next set of jobs when called again.
        """
        #print("getJobsAndAdvance\n")
        # handle any skipped dependencies, only when not doing the robust race checker.
        if not self.options.testharness_diagnostics:
            self._doSkippedDependencies()

        # else:
        #     for job in list(self.__job_dag.ind_nodes()):
        #         print(self.__job_dag.get_all_files(, self.netimes))
        #         change = self.__job_dag.check_changes(self.ortimes, self.netimes)
        #         self.ortimes = self.netimes
        # delete finished jobs
        next_jobs = set([])
        for job in list(self.__job_dag.ind_nodes()):
            if job.isFinished():
                next_jobs.add(job)
                self.__job_dag.delete_node(job)

        next_jobs.update(self.getJobs())


#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#        print(type(next_jobs))
#        self.__job_dag.get_all_files(next_jobs, self.netimes)
        return next_jobs

    def removeAllDependencies(self):
        """ Flatten current DAG so that it no longer contains any dependency information """
#        print("removeAllDependencies\n")
        if self.__name_to_job and self.__job_dag.size():
            tmp_job_dag = dag.DAG()
            for job in self.__job_dag.topological_sort():
                tmp_job_dag.add_node(job)
            self.__job_dag = tmp_job_dag
        return self.__job_dag

    def _checkDAG(self):
        """ perform some sanity checks on the current DAG """
#        print("_checkDAG\n")
        if self.__job_dag.size():

            self._doMakeDependencies()

            # print("\n\n\nWe are here.\n\n\n")
            # print(["Test {}, Prereqs {}".format(x.getTestName(), x.getPrereqs()) for x in self.__job_dag.topological_sort()])
            # print()
            # print(["Test {}, Predecessors {}".format(x.getTestName(), x.getUpstream(x)) for x in self.__job_dag.topological_sort()])
            # print()
            # print(["Test {}, Downsteams {}".format(x.getTestName(), x.getDownstreams(x)) for x in self.__job_dag.topological_sort()])
            #tester = job.getTester()
            #print(tester.getTestName())

            # We don't want to do _doSkippedDependencies if doing the robust checker.  It deletes edges.
#            if not self.options.testharness_diagnostics:
            self._doSkippedDependencies()

            # If there are race conditions, then there may be more skipped jobs
            # # If --diag is checked, run _doMakeSerializeDependencies
            # if self.options.testharness_diagnostics:
            #     ### If we run this, we need a copy of the DAG before the extra edges are added
            #     for name in self.__name_to_job:
            #         temp_job_list = self.__job_dag.all_downstreams(self.__name_to_job[name])
            #         for job in temp_job_list:
            #             self.__name_to_job[name].downstreamNodes.append(job.getTestName())

                #!!!!!!!!!!!! Pending removal !!!!!!!!!!!!#
                #self.__preSerializationDAG = self.__job_dag

                # for job in self.__job_dag.topological_sort():
                #     print(job.getUniqueIdentifier())
                #     stream = self.__job_dag.dprin(job)
                #     print("Downstream")
                #     for item in stream:
                #         print("", item.getUniqueIdentifier())
                # self._doMakeSerializeDependencies()
                # for job in self.__job_dag.topological_sort():
                #     print(job.getUniqueIdentifier())
                #     stream = self.__job_dag.dprin(job)
                #     print("Downstream")
                #     for item in stream:
                #         print("", item.getUniqueIdentifier())
                #print(self.__job_dag.get_all_files(jobs, self.ortimes))

            # We don't want to do _doRaceConditions or _doSkippedDependencies is we are robustly checking.
            if self._doRaceConditions():
                self._doSkippedDependencies()

            # If --diag is checked, run _doMakeSerializeDependencies
            if self.options.testharness_diagnostics:
                ### If we run this, we need a copy of the DAG before the extra edges are added
                for name in self.__name_to_job:
                    temp_job_list = self.__job_dag.all_downstreams(self.__name_to_job[name])
                    for job in temp_job_list:
                        self.__name_to_job[name].downstreamNodes.append(job.getTestName())

                self._doMakeSerializeDependencies()

            # print("\n\n\nWe are here.\n\n\n")
            # print(["Test {}, Prereqs {}".format(x.getTestName(), x.getPrereqs()) for x in self.__job_dag.topological_sort()])
            # print()
            # print(["Test {}, Predecessors {}".format(x.getTestName(), x.getUpstream(x)) for x in self.__job_dag.topological_sort()])
            # print()
            # print(["Test {}, Downsteams {}".format(x.getTestName(), x.getDownstreams(x)) for x in self.__job_dag.topological_sort()])

        return self.__job_dag

    def _doMakeSerializeDependencies(self):
        return self.__job_dag.serialize_dag()

    def _doMakeDependencies(self):
        """ Setup dependencies within the current Job DAG """
#        print("_doMakeDependencies\n\n\n")
        for job in self.__job_dag.ind_nodes():
            prereq_jobs = job.getUniquePrereqs()
#            print(prereq_jobs)
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

                ## If running the robust race checker, add the downstreams to the Job node
#                if self.options.testharness_diagnostics:



#############################################
#  Testing serialization
        # print("TOPO TEST")
        # for job in self.__job_dag.topological_sort():
        #     print(job.getUniqueIdentifier())
        #     stream = self.__job_dag.dprin(job)
        #     print("Downstream")
        #     for item in stream:
        #         print("", item.getUniqueIdentifier())
        # if not self.sorted:
        #     self.__job_dag.credgtop(self.__job_dag.topological_sort())
        #     self.sorted = 1
        #
        # print("\nsecond half")
        # for job in self.__job_dag.topological_sort():
        #     print(job.getUniqueIdentifier())
        #     stream = self.__job_dag.dprin(job)
        #     print("DownStream")
        #     for item in stream:
        #         print("", item.getUniqueIdentifier())
        #     print("")
#
###############################################

    def _doSkippedDependencies(self):
        """ Determine which jobs in the DAG should be skipped """
#        print("_doSkippedDependencies\n")
#        print("\n\n\n\nOne True Story\n\n\n\n\n")
        for job in list(self.__job_dag.topological_sort()):
            tester = job.getTester()
#            self.__job_dag.credgtop(self.__job_dag.topological_sort())

            ########################################
#            print(job.getUniqueIdentifier())

            dep_jobs = set([])
            if not job.getRunnable() or self._haltDescent(job):
                ##########################################
#                print("\n\nHalt!!\n\n")

                job.setStatus(job.skip)
                dep_jobs.update(self.__job_dag.all_downstreams(job))

                # Remove parent dependency so it can launch individually
                for p_job in self.__job_dag.predecessors(job):
#                    print("\n\n\n\n\nPred Bridge Go Bye-Bye\n\n\n\n\n\n\n")
                    self.__job_dag.delete_edge_if_exists(p_job, job)

            for d_job in dep_jobs:
                d_tester = d_job.getTester()
                ###################################
#                mnu = 0
                #####################################
#                print(mnu)
                #####################################
#                mnu = mnu + 1
#                print(d_job)#
#                print(d_tester)#
                if tester.isSilent() and not d_job.getRunnable():
                    d_tester.setStatus(d_tester.silent)
                elif not self._skipPrereqs():
                    d_job.setStatus(d_job.skip)
                    d_job.addCaveats('skipped dependency')

                self.__job_dag.delete_edge_if_exists(job, d_job)

    def _doRaceConditions(self):
        """ Check for race condition errors within in the DAG"""
#        print("_doRaceConditions\n")
        # Build output_file in relation to job dictionary
        l = self.__job_dag.topological_sort()
        output_to_job = {}
        ##########################################################################
#        print("\n\n\nTopo Sort")
        for job in l: #self.__job_dag.topological_sort():
            #############################################################################
#            print(job.getUniqueIdentifier())
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
        #print("_haltDescent")
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
#        print("_skipPrereqs\n")
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
#        print("_printDownstreams\n")
        downstreams = self.__job_dag.all_downstreams(job)
        cyclic_path = []
        for d_job in downstreams:
            cyclic_path.append('%s -->'% (d_job.getTestNameShort()))
        return ' '.join(cyclic_path)
