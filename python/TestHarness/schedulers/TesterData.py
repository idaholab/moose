import re, os
from timeit import default_timer as clock

import util

# TODO: Figure out what to do with this entire class... The issue is, some of it belongs in the Tester, some
#       of it in the Scheduler. But no one wants the entire thing.
class TesterData:
    """
    The TesterData class is a simple container for the tester and its associated output file object, the DAG,
    the process object, the exit codes, the start and end time, etc
    """
    def __init__(self, tester, dag_object, options):
        self.options = options
        self.__tester = tester
        self.__dag = dag_object
        self.__outfile = None
        self.__process = None
        self.__exit_code = None
        self.__start_time = None
        self.__end_time = None
        self.__std_out = ''

    # Return the tester object
    def getTester(self):
        return self.__tester

    # Return the DAG object associated with this tester
    def getDAG(self):
        return self.__dag

    # Run a tester specific command (changes into test_dir before execution)
    def runCommand(self, command):
        if self.options.dry_run or not self.__tester.shouldExecute():
            self.__tester.setStatus(self.__tester.getSuccessMessage(), self.__tester.bucket_success)
            self.__exit_code = 0
            self.__start_time = clock()
            self.__end_time = clock()
            return

        # Run a command and get the process and tempfile
        (process, output_file) = util.launchCommand(self.__tester, command)

        # Save this information before waiting, so
        self.__process = process
        self.__outfile = output_file
        self.__start_time = clock()

        # Wait for the process to finish
        process.wait()

        # Record some information about this finished command
        self.__end_time = clock()
        self.__exit_code = process.poll()

        # This saves the trimmed output to std_out, and closes the file
        self.__std_out = self.getOutput()

        # Return the process
        return process

    # Kill the process
    def killProcess(self):
        if self.__process:
            self.__process.kill()

    # Return Exit code
    def getExitCode(self):
        return self.__exit_code

    # Return start time
    def getStartTime(self):
        return self.__start_time

    # Return end time
    def getEndTime(self):
        return self.__end_time

    # Return the output file handler object
    def getOutFile(self):
        return self.__outfile

    # Close the OutFile object
    def closeFile(self):
        if not self.__outfile.closed:
            self.__outfile.close()

    ######
    # Set or override the output
    # NOTE: This does nothing if there is a valid file handler and its opened (a running non-skipped
    #       test). This is to protect the output that the running process is busy creating.
    #
    #       Proper usage would be to; wait for procsess to finish, getOutput(), modify it, and then
    #       setOutput(<modified output>). Concurrent getOutput()'s would then return your modified
    #       output.
    def setOutput(self, output):
        if self.__outfile is not None and not self.__outfile.closed:
            return
        self.__std_out = output

    # Read and return the output, then close the file
    def getOutput(self):
        if self.__outfile is not None and not self.__outfile.closed:
            self.__std_out = util.readOutput(self.__outfile, self.options)
            self.__outfile.close()
        return self.__std_out

    # Return active time
    def getActiveTime(self):
        m = re.search(r"Active time=(\S+)", self.__std_out)
        if m != None:
            return m.group(1)

    # Return solve time
    def getSolveTime(self):
        m = re.search(r"solve().*", self.__std_out)
        if m != None:
            return m.group().split()[5]

    # Return active time if available, if not return a comparison of start and end time
    def getTiming(self):
        if self.getActiveTime():
            return self.getActiveTime()
        elif self.getEndTime() and self.getStartTime():
            return self.getEndTime() - self.getStartTime()
        else:
            return 0.0
