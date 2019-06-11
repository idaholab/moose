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

    def findRacePartners(self):
        """ Collect all the jobs that have race conditions """
        raceConditionsExist = False
        for job_group in self.all_jobs:
            for job_a, job_b in itertools.combinations(job_group, 2):
                if job_a.isSkip() or job_b.isSkip():
                    continue

                _matching = set(job_a.modifiedFiles).intersection(set(job_b.modifiedFiles))
                if _matching and not ((job_a in job_b.getDownstreamNodes()) \
                                      or (job_b in job_a.getDownstreamNodes())):
                    job_a.racePartners.add(job_b)
                    job_b.racePartners.add(job_a)
                    raceConditionsExist = True
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

        allOfRacers = set()
        for job_group in self.all_jobs:
            temp_set = set()
            for job in job_group:

                # Only jobs that have race conditions
                if not len(job.racePartners):
                    continue

                temp_set.add(job)
                for partner in job.racePartners:
                    temp_set.add(partner)
                allOfRacers.add(frozenset(sorted(temp_set)))
        setNumber = 0
        # Keep the files that are modified.
        racerModifiedFiles = set()

        # If there are any jobs with race conditions, print them out.
        if len(allOfRacers) > 0:
            print("\nDiagnostic analysis shows that the members of the following unique sets exhibit race conditions:")

            # Print each set of jobs with shared race conditions.
            for racers in allOfRacers:
                racerModifiedFiles.clear()
                if racers:
                    setNumber += 1
                    print(" Set " + str(setNumber) + "\n- - - - -")
                    for racer in racers:
                        print("  --" + racer.getTestName())
                    for a, b in itertools.combinations(racers, 2):
                        for c in a.modifiedFiles:
                            if c in b.modifiedFiles:
                                racerModifiedFiles.add(c)
                            else:
                                try:
                                    racerModifiedFiles.remove(c)
                                except:
                                    pass
                    print("\n   Each of the tests in this set create or modify each of the following files:")
                    for file in racerModifiedFiles:
                        print("    -->" + file)
                    print("- - - - -\n\n")

            if setNumber > 1:
                print("There are a total of " + str(setNumber) + " sets of tests with unique race conditions. ")
            else:
                print("There is " + str(setNumber) + " set of tests with unique race conditions. ")
            print("Please review the tests and either add any necessary prereqs, or create unique filenames " +
            "for the outputs of each test.")
            exit_code = status.race.code

        else:
            print("There are no race conditions.")

        return exit_code
