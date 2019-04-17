#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re, os, json
from timeit import default_timer as clock
from TestHarness import StatusSystem


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
    def __init__(self, tester, job_dag, options):
        self.options = options
        self.__tester = tester
        self.__job_dag = job_dag
        self.timer = Timer()
        self.__outfile = None
        self.__start_time = clock()
        self.__end_time = None
        self.__previous_time = None
        self.__joined_out = ''
        self.report_timer = None
        self.__slots = None
        self.__meta_data = {}

        # Enumerate available job statuses
        self.status = StatusSystem.JobStatus()

        self.hold = self.status.hold
        self.queued = self.status.queued
        self.running = self.status.running
        self.skip = self.status.skip
        self.crash = self.status.crash
        self.error = self.status.error
        self.timeout = self.status.timeout
        self.finished = self.status.finished

    def getDAG(self):
        """ Return the DAG associated with this tester """
        return self.__job_dag

    def getTester(self):
        """ Return the tester object """
        return self.__tester

    def getTestName(self):
        """ Wrapper method to return the testers test name """
        return self.__tester.getTestName()

    def getTestNameShort(self):
        """ Return the shorthand Test name """
        return self.getTestName().split('.')[1]

    def getTesterStatus(self):
        """ Wrapper method to return a testers current status """
        return self.__tester.getStatus()

    def getPrereqs(self):
        """ Wrapper method to return the testers prereqs """
        return self.__tester.getPrereqs()

    def getTestDir(self):
        """ Wrapper method to return the testers working directory """
        return self.__tester.getTestDir()

    def addCaveats(self, kwargs):
        """ Wrapper method for setting caveats """
        return self.__tester.addCaveats(kwargs)

    def getCaveats(self):
        """ Wrapper method for getting caveats """
        return self.__tester.getCaveats()

    def clearCaveats(self):
        """ Wrapper method for clearing caveats """
        return self.__tester.clearCaveats()

    def getRunnable(self):
        """ Wrapper method to return getRunnable """
        return self.__tester.getRunnable(self.options)

    def getOutputFiles(self):
        """ Wrapper method to return getOutputFiles """
        return self.__tester.getOutputFiles()

    def getUniqueIdentifier(self):
        """ A unique identifier for this job object """
        return os.path.join(self.getTestDir(), self.getTestName())

    def getUniquePrereqs(self):
        """ Return a list of prereqs with what should be their unique identifier """
        prereqs = self.getPrereqs()
        unique_prereqs = []
        for prereq in prereqs:
            unique_prereqs.append(os.path.join(self.getTestDir(), prereq))
        return unique_prereqs

    def addMetaData(self, **kwargs):
        """ Allow derived methods to store additional data which ends up in the data storage file """
        for key, value in kwargs.iteritems():
            self.__meta_data[key] = value

    def getMetaData(self):
        """ return data stored by addMetaData """
        return self.__meta_data

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

        # Do not execute app, but allow processResults to commence
        if not self.__tester.shouldExecute():
            return

        self.__tester.prepare(self.options)

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

        # Check for invalid unicode in output
        try:
            json.dumps(output)

        except UnicodeDecodeError:
            # convert invalid output to something json can handle
            output = output.decode('utf-8','replace').encode('ascii', 'replace')

            # Alert the user that output has invalid characters
            self.addCaveats('invalid characters in stdout')

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

    def setPreviousTime(self, t):
        """
        Allow an arbitrary time to be set. This is used by the QueueManager
        to set the time as recorded by a previous TestHarness instance.
        """
        self.__previous_time = t

    def getTiming(self):
        """ Return active time if available, if not return a comparison of start and end time """
        if self.getActiveTime():
            return self.getActiveTime()
        elif self.getEndTime() and self.getStartTime():
            return self.timer.cumulativeDur()
        elif self.getStartTime() and self.isRunning():
            # If the test is still running, return current run time instead
            return max(0.0, clock() - self.getStartTime())
        elif self.__previous_time:
            return self.__previous_time
        else:
            return 0.0

    # Wrapper methods for adjusting statuses
    def getStatus(self):
        return self.status.getStatus()
    def getStatusMessage(self):
        return self.status.getStatusMessage()
    def getStatusCode(self):
        return self.status.getStatusCode()
    def setStatus(self, status, message=''):
        return self.status.setStatus(status, message)

    # Wrapper methods for returning a bool status
    def isHold(self):
        return self.status.isHold()
    def isSkip(self):
        return self.status.isSkip()
    def isQueued(self):
        return self.status.isQueued()
    def isRunning(self):
        return self.status.isRunning()
    def isCrash(self):
        return self.status.isCrash()
    def isError(self):
        return self.status.isError()
    def isFail(self):
        return self.status.isFail()
    def isFinished(self):
        return self.status.isFinished()
    def isTimeout(self):
        return self.status.isTimeout()
