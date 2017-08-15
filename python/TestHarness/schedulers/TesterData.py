import re, os, platform
from timeit import default_timer as clock
from signal import SIGTERM
import util

class TesterData(object):
    """
    The TesterData class is a simple container for the tester and its associated output file object, the DAG,
    the process object, the exit codes, and the start and end times.
    """
    def __init__(self, tester, tester_dag, options):
        self.options = options
        self.__tester = tester
        self.__dag = tester_dag
        self.__outfile = None
        self.__process = None
        self.__exit_code = 0
        self.__start_time = clock()
        self.__end_time = None
        self.__std_out = ''
        self.report_timer = None

    def getTester(self):
        """ Return the tester object """
        return self.__tester

    def getDAG(self):
        """ Return the DAG object """
        return self.__dag

    def getTestName(self):
        """ Wrapper method to return the testers test name """
        return self.__tester.getTestName()

    def processCommand(self, command):
        """
        A blocking method to handle the exit status of the process object while keeping track of the
        time the process was active. When the process exits, read the output and close the file.
        """

        if self.options.dry_run or not self.__tester.shouldExecute():
            self.__tester.setStatus(self.__tester.getSuccessMessage(), self.__tester.bucket_success)
            return

        # Run the command and get the process and tempfile
        (process, output_file) = util.launchTesterCommand(self.__tester, command)

        self.__process = process
        self.__outfile = output_file
        self.__start_time = clock()

        process.wait()

        self.__end_time = clock()
        self.__exit_code = process.poll()

        # store the contents of output, and close the file
        self.__std_out = util.readOutput(self.__outfile, self.options)
        self.__outfile.close()

        return process

    def killProcess(self):
        """ Kill remaining process that may be running """

        if self.__process is not None:
            try:
                if platform.system() == "Windows":
                    self.__process.terminate()
                else:
                    pgid = os.getpgid(self.__process.pid)
                    os.killpg(pgid, SIGTERM)
            except OSError: # Process already terminated
                pass

    def getExitCode(self):
        """ Return exit code """
        return self.__exit_code

    def getStartTime(self):
        """ Return the time the process started """
        return self.__start_time

    def getEndTime(self):
        """ Return the time the process exited """
        return self.__end_time

    def getOutput(self):
        """ Return the contents of output """
        return self.__std_out

    def setOutput(self, output):
        """ Method to allow testers to overwrite the output if certain conditions are met """
        if self.__outfile is not None and not self.__outfile.closed:
            return
        self.__std_out = output

    def getActiveTime(self):
        """ Return active time """
        m = re.search(r"Active time=(\S+)", self.__std_out)
        if m != None:
            return m.group(1)

    def getSolveTime(self):
        """ Return solve time """
        m = re.search(r"solve().*", self.__std_out)
        if m != None:
            return m.group().split()[5]

    def getTiming(self):
        """ Return active time if available, if not return a comparison of start and end time """
        if self.getActiveTime():
            return self.getActiveTime()
        elif self.getEndTime() and self.getStartTime():
            return self.getEndTime() - self.getStartTime()
        elif self.getStartTime():
            # If the test is still running, return current run time instead
            return max(0.0, clock() - self.getStartTime())
        else:
            return 0.0
