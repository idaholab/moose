#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, threading, time, traceback
from collections import namedtuple
from TestHarness import OutputInterface, util

class Runner(OutputInterface):
    # Helper struct for storing information about sampled resource usage
    ResourceUsage = namedtuple('ResourceUsage', 'time mem_bytes')

    """
    Base class for running a process via a command.

    Used within the Tester to actually run a test. We need
    this specialized so that we can either run things locally
    or externally (i.e., PBS, slurm, etc on HPC)
    """
    def __init__(self, job, options):
        # Output is locking so that the resource thread can concurrently write
        OutputInterface.__init__(self, locking=True)

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
        run_output = self.getRunOutput()
        for file_path in self.job.getTester().getRedirectedOutputFiles(self.options):
            run_output.appendOutput(util.outputHeader(f'Begin redirected output {file_path}'))
            if os.access(file_path, os.R_OK):
                with open(file_path, 'r+b') as f:
                    run_output.appendOutput(self.readOutput(f))
            else:
                self.job.setStatus(self.job.error, 'FILE TIMEOUT')
                self.appendOutput(f'File {file_path} unavailable')
            run_output.appendOutput(util.outputHeader(f'End redirected output {file_path}'))

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

    def getResourceUsage(self):
        """
        To be overridden by derived Runners that support resource usage collection

        Should return a list of ResourceUsage objects
        """
        return None

    def getMaxMemoryUsage(self):
        """
        Get the max memory usage (in bytes) of the spawned process if it was
        able to be captured
        """
        resource_usage = self.getResourceUsage()
        if not resource_usage: # runner doesn't support it
            return None
        max_mem = 0
        for usage in resource_usage:
            max_mem = max(max_mem, usage.mem_bytes)
        return max_mem

    def checkResourceUsage(self, usage):
        """
        Checks the resource usage to ensure that it does not go over
        limits. Will kill the job if so.

        Usage should be a ResourceUsage object
        """
        # Scale all of the requirements on a per-slot basis
        slots = self.job.getSlots()

        # Check for memory overrun if max is set
        if self.options.max_memory is not None:
            allowed_mem = slots * self.options.max_memory
            if usage.mem_bytes > allowed_mem:
                usage_human = util.humanMemory(usage.mem_bytes)
                allowed_human = util.humanMemory(allowed_mem)
                output = util.outputHeader('Process killed due to resource oversubscription')
                output += f'Memory usage {usage_human} exceeded {allowed_human}'
                self.appendOutput(output)
                self.job.setStatus(self.job.error, 'EXCEEDED MEM')
                self.kill()
                return
