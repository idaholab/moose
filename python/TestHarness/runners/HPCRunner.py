#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re, time, os, subprocess, yaml
from TestHarness.runners.Runner import Runner
from TestHarness import util

class HPCRunner(Runner):
    """
    Base Runner to be used with HPC schedulers (PBS, slurm)
    """
    def __init__(self, job, options, run_hpc):
        super().__init__(job, options)

        # The RunHPC object
        self.run_hpc = run_hpc

        # The HPC job, set during spawn()
        self.hpc_job = None

        # Interval in seconds for polling for job status
        self.job_status_poll_time = 0.1

        # Interval in seconds for polling for file completion
        self.file_completion_poll_time = 0.1

    def spawn(self, timer):
        # The runner_run output, which is the output from what we're
        # actually running, already exists as a file. So just load
        # it from that file instead and don't bother loading it
        # into memory
        hpc_job_output_path = self.run_hpc.getHPCJobOutputPath(self.job)
        self.getRunOutput().setSeparateOutputPath(hpc_job_output_path)

        # Rely on the RunHPC object to queue the job
        self.hpc_job = self.run_hpc.queueJob(self.job)

    def wait(self, timer):
        # Sanity check on having a job
        if self.hpc_job is None:
            self.job.setStatus(self.job.error, 'HPCRUNNER MISSING HPCJOB')
            return

        # The states that we should wait on. Anything else should
        # be an invalid state for waiting
        wait_states = [self.hpc_job.State.held,
                       self.hpc_job.State.queued,
                       self.hpc_job.State.running]

        # Poll loop waiting for the job to be finished
        # This gets a structure that represents the job, and the
        # polling itself is only done on occasion within RunHPC
        while True:
            time.sleep(self.job_status_poll_time)
            with self.hpc_job.getLock():
                if self.hpc_job.state not in wait_states:
                    self.exit_code = self.hpc_job.exit_code
                    break

        # The PBS output (stdout+stderr)
        output_file = self.run_hpc.getHPCJobOutputPath(self.job)
        # The result file (exit code + walltime)
        result_file = self.run_hpc.getHPCJobResultPath(self.job)

        # If the Job is already finished, something happened in the
        # HPC scheduler so we have an invalid state for processing
        if self.job.isFinished():
            return

        tester = self.job.getTester()

        # Determine the output files that we need to wait for to be complete
        wait_files = set([output_file, result_file])
        # Output files needed by the Tester, only if it says we should
        if tester.mustOutputExist(self.exit_code):
            wait_files.update(self.job.getOutputFiles(self.options))
        # The files that we can read, but are incomplete (no terminator)
        incomplete_files = set()

        # Wait for all of the files to be available
        timer.start('hpc_wait_output')
        waited_time = 0
        walltime = None
        while wait_files or incomplete_files:
            # Don't bother if we've been killed
            if self.hpc_job.isKilled():
                return

            # Look for each file
            for file in wait_files.copy():
                if os.path.exists(file) and os.path.isfile(file):
                    wait_files.discard(file)
                    incomplete_files.add(file)

            # Check for file completeness
            for file in incomplete_files.copy():
                if self.fileIsReady(file):
                    # Store the result
                    if file == result_file:
                        try:
                            with open(file, 'r') as f:
                                result = yaml.safe_load(f)
                        except:
                            continue
                        self.exit_code = result['exit_code']
                        walltime = result['walltime']

                        # Delete this, we don't really need it to hang around
                        try:
                            os.remove(file)
                        except OSError:
                            pass

                    # Done with this file
                    incomplete_files.discard(file)

            # We've waited for files for too long
            if (wait_files or incomplete_files) and waited_time >= self.options.hpc_file_timeout:
                self.job.setStatus(self.job.timeout, 'FILE TIMEOUT')
                def print_files(files, type):
                    if files:
                        self.appendOutput(f'{type} output file(s)\n')
                        self.appendOutput('\n'.join(files) + '\n')
                print_files(wait_files, 'Unavailable')
                print_files(incomplete_files, 'Incomplete')
                break

            waited_time += self.file_completion_poll_time
            time.sleep(self.file_completion_poll_time)
        timer.stop('hpc_wait_output')

        # If we have a walltime from output, use it instead as it'll be
        # more accurate for the real runtime
        if walltime:
            timer = self.job.timer
            start_time = timer.startTime('runner_run')
            end_time = start_time + walltime
            timer.reset('runner_run')
            timer.start('runner_run', start_time)
            timer.stop('runner_run', end_time)

        # Handle openmpi appending a null character at the end of jobs
        # that return a nonzero exit code. An example of this is:
        #
        # --------------------------------------------------------------------------
        # MPI_ABORT was invoked on rank 0 in communicator MPI_COMM_WORLD
        #   Proc: [[PID,1],0]
        #   Errorcode: 1
        #
        # NOTE: invoking MPI_ABORT causes Open MPI to kill all MPI processes.
        # You may or may not see output from other processes, depending on
        # exactly when Open MPI kills them.
        # --------------------------------------------------------------------------
        # <NULL>--------------------------------------------------------------------------
        # prterun has exited due to process rank 0 with PID 0 on node HOSTNAME calling
        # "abort". This may have caused other processes in the application to be
        # terminated by signals sent by prterun (as reported here).
        # --------------------------------------------------------------------------
        # <NULL>
        #
        # Where <NULL> is there the null character ends up. Thus, in cases
        # where we have a nonzero exit code and a MPI_ABORT, we'll try to remove these.
        if self.exit_code != 0 and self.job.getTester().hasOpenMPI():
            output = self.getRunOutput().getOutput(sanitize=False)
            if 'MPI_ABORT' in output:
                output_changed = False
                if output:
                    for null in ['\0', '\x00']:
                        prefix = '-'*74 + '\n'
                        prefix_with_null = prefix + null
                        if prefix_with_null in output:
                            output = output.replace(prefix_with_null, prefix, 1)
                            output_changed = True
                if output_changed:
                    self.getRunOutput().setOutput(output)


    def kill(self):
        if self.hpc_job:
            self.run_hpc.killHPCJob(self.hpc_job)

    def fileIsReady(self, file):
        """
        Checks if a file is ready for reading.

        In summary:
        - Check if the file exists
        - If the file exists, make sure that it has the terminator
          string (to know that we have the full file)
        - Remove the terminator string
        """
        # The file terminator check (to we have the up-to-date copy of the file)
        # is dependent on whether or not the file is a binary
        is_binary = self.isFileBinary(file)
        # If this returns None, it means that the "file" command couldn't determine
        # the file type, which may be the case if we have an incomplete file so
        # just continue and check on the next iteration
        if is_binary is None:
            return False

        ending_comment = self.run_hpc.getOutputEndingComment(self.hpc_job.id)

        # Binary file
        if is_binary:
            with open(file, "rb+") as file:
                # We'll be looking for this many characters
                len_comment = len(ending_comment)

                # Move to the end and figure out the position
                # back where our terminator should be
                file.seek(0, os.SEEK_END)
                pos = file.tell() - len_comment

                # File is shorter than our comment
                if pos < 0:
                    return False

                # Move to the position where our terminator _should_ be
                file.seek(pos)

                # We try here in the event that we're loading
                # an earlier part of the file and we can't decode
                try:
                    contents = file.read(len_comment).decode('utf-8')
                except:
                    return False

                # Terminator isn't there
                if contents != ending_comment:
                    return False

                # Remove the terminator
                file.seek(pos)
                file.truncate()

                return True
        # Text file
        else:
            line, pos = self.getLastLine(file)
            if ending_comment == line:
                with open(file, "r+", encoding="utf-8") as f:
                    f.seek(pos)
                    f.truncate()
                return True

        return False

    @staticmethod
    def isFileBinary(file):
        """
        Returns whether or not the given file is a binary file.

        If None, a failure was encountered when checking the file type.
        """
        try:
            call_file = subprocess.check_output(['file', '--mime-encoding', file], text=True)
        except:
            return None

        # Will return something like "<filename>: <encoding>",
        # where <encoding>=binary when the file is binary
        find_binary = re.search('binary$', call_file)
        return find_binary is not None

    @staticmethod
    def getLastLine(file):
        """
        Gets the last line of a text file and the position
        in the file at which that last line is.
        """
        with open(file, 'rb') as f:
            try:
                f.seek(-2, os.SEEK_END)
                while f.read(1) != b'\n':
                    f.seek(-2, os.SEEK_CUR)
            except OSError: # one line filecd
                f.seek(0)
            pos = f.tell()
            line = f.readline().decode('utf-8')
            return line, pos
