#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, json
from TestHarness import OutputInterface

class Runner(OutputInterface):
    """
    Base class for running a process via a command.

    Used within the Tester to actually run a test. We need
    this specialized so that we can either run things locally
    or externally (i.e., PBS, slurm, etc on HPC)
    """
    def __init__(self, job, options):
        OutputInterface.__init__(self)

        # The job that this runner is for
        self.job = job
        # The test harness options
        self.options = options
        # The job's exit code, should be set after wait()
        self.exit_code = None
        # The output for the actual run of the job. We keep this
        # separate from self.output in this Runner because HPC
        # jobs always have a file output, so we want to store
        # their output separately
        self.run_output = OutputInterface()

    def getRunOutput(self):
        """ Get the OutputInterface object for the actual run """
        return self.run_output

    def spawn(self, timer):
        """
        Spawns the process.

        Wait for the process to complete with wait().

        Should be overridden.
        """
        pass

    def wait(self, timer):
        """
        Waits for the process started with spawn() to complete.

        Should be overridden.
        """
        pass

    def kill(self):
        """
        Kills the process started with spawn()

        Should be overridden.
        """
        pass

    def finalize(self):
        """
        Finalizes the output, which should be called at the end of wait()
        """
        # Load the redirected output files, if any
        for file_path in self.job.getTester().getRedirectedOutputFiles(self.options):
            self.appendOutput(util.outputHeader(f'Begin redirected output {file_path}'))
            if os.access(file_path, os.R_OK):
                with open(file_path, 'r+b') as f:
                    self.appendOutput(self.readOutput(f))
            else:
                self.job.setStatus(self.job.error, 'FILE TIMEOUT')
                self.appendOutput('FILE UNAVAILABLE\n')
            self.appendOutput(util.outputHeader(f'End redirected output {file_path}'))

        # Check for invalid unicode in output
        output = self.getOutput()
        try:
            json.dumps(output)
        except UnicodeDecodeError:
            # Convert invalid output to something json can handle
            self.setOutput(output.decode('utf-8','replace').encode('ascii', 'replace'))
            # Alert the user that output has invalid characters
            self.job.addCaveats('invalid characters in output')

        # Remove NULL output and fail if it exists
        output = self.getOutput()
        null_chars = ['\0', '\x00']
        for null_char in null_chars:
            if null_char in output:
                self.setOutput(output.replace(null_char, 'NULL'))
                self.job.setStatus(self.job.error, 'NULL characters in output')

    def getExitCode(self):
        """
        Gets the error code of the process.
        """
        return self.exit_code

    def sendSignal(self, signal):
        """
        Sends a signal to the process.

        Can be overridden.
        """
        raise Exception('sendSignal not supported for this Runner')

    def readOutput(self, stream):
        """
        Helper for reading output from a stream, and setting an error state
        if the read failed.
        """
        output = ''
        try:
            stream.seek(0)
            output = stream.read().decode('utf-8')
        except UnicodeDecodeError:
            self.job.setStatus(self.job.error, 'non-unicode characters in output')
        except:
            self.job.setStatus(self.job.error, 'error reading output')
        if output and output[-1] != '\n':
            output += '\n'
        return output
