#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys, itertools
from StatusSystem import StatusSystem  # for proper error code

class RaceChecker(object):
    """
    Class with methods to check if two jobs have modified a file with the same name
    while not having the proper dependencies to avoid race conditions.
    """

    def __init__(self, all_jobs):
        """ Create a RaceChecker object that has all the jobs"""
        self.all_jobs = all_jobs
        self.racer_lists = []

    def findRacePartners(self):
        """ Collect all the jobs that have race conditions """
        raceConditionsExist = False
        for job_group in self.all_jobs:
            _jobs = set([])
            for job_a, job_b in itertools.combinations(job_group, 2):
                if job_a.isSkip() or job_b.isSkip():
                    continue

                _matching = list(set(job_a.modifiedFiles).intersection(set(job_b.modifiedFiles)))
                if _matching and not ((job_a in job_b.getDownstreamNodes()) \
                                      or (job_b in job_a.getDownstreamNodes())):
                    _jobs.update([job_a, job_b])
                    raceConditionsExist = True
            if _jobs:
                self.racer_lists.append((sorted(_jobs), sorted(_matching)))
        return raceConditionsExist

    def printRaceConditionsByPrereq(self):
        """ Print jobs with race conditions that share a prereq """

        colissions = dict()
        for job in self.all_jobs:
            if len(job.racePartners) > 0:
                prereq = str([x.getTestName() for x in job.getUpstreamNodes()])
                shared = list()
                shared.append(job.getTestName())
                for partner in job.racePartners:
                    if job.getUpstreamNodes() == partner.getUpstreamNodes():
                        shared.append(partner.getTestName())
                    colissions[prereq] = shared
        for prereq in colissions:
            if prereq != "[]":
                print("The following share " + prereq + " for their prereq(s)")
            else:
                print("The following don't share any prereqs")
            for job in colissions[prereq]:
                print(job)
        return

    def printUniqueRacerSets(self):
        """ Print the jobs that share race conditions within unique sets """
        status = StatusSystem()
        exit_code = 0x0
        if self.racer_lists:
            output = ""
            for i, racers in enumerate(self.racer_lists):
                job_list, file_matches = racers
                output += "\nSet %d\n" % (i+1)
                output += "- "*4 + "\n\t--%s" % ('\n\t--'.join([x.getTestName() for x in job_list]))
                output += "\n\nEach of the tests in this set create or modify each of the following files:"
                output += "\n\t-->%s" % ('\n\t-->'.join(file_matches))
                output += "\n" + "- "*4
            exit_code = status.race.code
        if output:
            output += "\nThere are a total of %d sets of tests with unique race conditions." % (i+1)
            output += "\nPlease review the tests and either add any necessary prereqs, or create unique filenames for the outputs of each test."
            print(output)
        return exit_code
