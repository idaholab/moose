#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re, os
from timeit import default_timer as clock


class Timer(object):
    """
    A helper class for testers to track the time it takes to run.

    Every call to the start method must be followed by a call to stop.
    """
    def __init__(self):
        self.starts = []
        self.ends = []
    def start(self):
        """ starts the timer clock """
        self.starts.append(clock())
    def stop(self):
        """ stop/pauses the timer clock """
        self.ends.append(clock())
    def cumulativeDur(self):
        """ returns the total/cumulative time taken by the timer """
        diffs = [end - start for start, end in zip(self.starts, self.ends)]
        return sum(diffs)
    def avgerageDur(self):
        return self.cumulativeDur() / len(self.starts)
    def nRuns(self):
        return len(self.starts)
    def reset(self):
        self.starts = []
        self.ends = []

class Job(object):
    """
    The Job class is a simple container for the tester and its associated output file object, the DAG,
    the process object, the exit codes, and the start and end times.
    """
    def __init__(self, tester, tester_dag, options):
        self.options = options
        self.__tester = tester
        self.timer = Timer()
        self.__dag = tester_dag
        self.__outfile = None
        self.__start_time = clock()
        self.__end_time = None
        self.__joined_out = ''
        self.report_timer = None
        self.__slots = None
        self.__unique_identifier = os.path.join(tester.getTestDir(), tester.getTestName())

    def getTester(self):
        """ Return the tester object """
        return self.__tester

    def getDAG(self):
        """ Return the DAG object """
        return self.__dag

    def getOriginalDAG(self):
        """ Return the DAG object as it was in its original form """
        return self.__dag.getOriginalDAG()

    def getTestName(self):
        """ Wrapper method to return the testers test name """
        return self.__tester.getTestName()

    def getUniqueIdentifier(self):
        """ A unique identifier for this job object """
        return self.__unique_identifier

    def getSlots(self):
        """ Return the number of slots this job consumes """
        if self.__slots == None:
            return self.setSlots(self.__tester.getSlots(self.options))
        return self.__slots

    def setSlots(self, slots):
        """ Set the number of processors this job consumes """
        self.__slots = int(slots)
        return self.__slots

    def run(self):
        """
        A blocking method to handle the exit status of the process object while keeping track of the
        time the process was active. When the process exits, read the output and close the file.
        """
        self.__tester.prepare(self.options)

        if self.options.dry_run or not self.__tester.shouldExecute():
            self.__tester.setStatus(self.__tester.getSuccessMessage(), self.__tester.bucket_success)
            return

        self.__start_time = clock()
        self.timer.reset()
        self.__tester.run(self.timer, self.options)
        self.__start_time = self.timer.starts[0]
        self.__end_time = self.timer.ends[-1]
        self.__joined_out = self.__tester.joined_out

    def killProcess(self):
        """ Kill remaining process that may be running """
        self.__tester.killCommand()

    def getStartTime(self):
        """ Return the time the process started """
        return self.__start_time

    def getEndTime(self):
        """ Return the time the process exited """
        return self.__end_time

    def getOutput(self):
        """ Return the contents of output """
        return self.__joined_out

    def setOutput(self, output):
        """ Method to allow schedulers to overwrite the output if certain conditions are met """
        if (not self.__tester.outfile is None and not self.__tester.outfile.closed
           and not self.__tester.errfile is None and not self.__tester.errfile.closed):
            return
        self.__joined_out = output

    def getActiveTime(self):
        """ Return active time """
        m = re.search(r"Active time=(\S+)", self.__joined_out)
        if m != None:
            return m.group(1)

    def getSolveTime(self):
        """ Return solve time """
        m = re.search(r"solve().*", self.__joined_out)
        if m != None:
            return m.group().split()[5]

    def getTiming(self):
        """ Return active time if available, if not return a comparison of start and end time """
        if self.getActiveTime():
            return self.getActiveTime()
        elif self.getEndTime() and self.getStartTime():
            return self.timer.cumulativeDur()
        elif self.getStartTime() and self.__tester.isPending():
            # If the test is still running, return current run time instead
            return max(0.0, clock() - self.getStartTime())
        else:
            return 0.0
