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

        # Whether or not the primary output has been loaded fully
        self.output_completed = False

    def spawn(self, timer):
        # Rely on the RunHPC object to queue the job
        self.hpc_job = self.run_hpc.queueJob(self.job)

    def wait(self, timer):
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
        # in the Tester
        if self.job.isFinished():
            # Don't bother if we've been killed
            if self.hpc_job.isKilled():
                return

            # If we have _some_ output, at least try to load it. However, don't wait
            # a while for this one.
            for i in range(int(60 / self.file_completion_poll_time)):
                if self.trySetOutput():
                    break
                time.sleep(self.file_completion_poll_time)

            # Don't bother looking for the rest of the output
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
                    # Store the output
                    if file == output_file:
                        # It's now required because its complete
                        if not self.trySetOutput(required=True):
                            break
                    # Store the result
                    elif file == result_file:
                        with open(file, 'r') as f:
                            result = yaml.safe_load(f)
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
                if not self.output_completed:
                    self.trySetOutput()
                def print_files(files, type):
                    if files:
                        self.appendOutput(util.outputHeader(f'{type} output file(s):', ending=False))
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
        # that return a nonzero exit code. We don't know how to fix this
        # in openmpi yet, so this is the cleanest way to take care of it.
        # We're looking for \n\0##########', which is at the end of the
        # apptainer execution within hpc_template. This allows the null
        # character check that happens in Runner.finalize() to still
        # be valid.
        if self.exit_code != 0 and self.job.getTester().hasOpenMPI():
            output = self.getOutput()
            if output:
                prefix = '\n'
                null = '\0'
                suffix = '##########'
                all = f'{prefix}{null}{suffix}'
                no_null = f'{prefix}{suffix}'
                if all in output:
                    self.setOutput(output.replace(all, no_null))

    def kill(self):
        if self.hpc_job:
            self.run_hpc.killHPCJob(self.hpc_job)

    def trySetOutput(self, required=False):
        """
        Tries to set the output if it exists.

        If required is set, this will fail the job.

        Returns whether or not the output was set.
        """
        # Whether or not we actually set it
        did_set = False

        output_file = self.run_hpc.getHPCJobOutputPath(self.job)
        if os.path.exists(output_file) and os.path.isfile(output_file):
            try:
                header = f'{self.run_hpc.getHPCSchedulerName()} job {self.hpc_job.id} output'
                # If we're trying to parse output, we can't truncate it
                # because it might appear in the truncated portion
                if self.job.getTester().needFullOutput(self.options) or self.job.hasSeperateOutput():
                    self.setOutput(open(output_file, 'r').read())
                # Not parsing the output, so just read it truncated
                else:
                    self.setOutput(self.readTruncated(output_file))

                did_set = True
            except:
                pass

        if did_set:
            self.output_completed = True
        else:
            self.setOutput(f'Failed to load output file {output_file}\n')
            if required:
                self.job.setStatus(self.job.error, 'FAILED OUTPUT READ')

        return did_set

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

    @staticmethod
    def readTruncated(file, start_lines=500, end_lines=500):
        """
        Reads a file and truncates it past a certain amount of lines.
        """
        with open(file, 'rb') as f:
            # Find the end position of the file so that we don't read past
            f.seek(0, os.SEEK_END)
            total_bytes = f.tell()

            # Read the set of lines
            f.seek(0)
            head_lines_read = 0
            head = ''
            while head_lines_read < start_lines and f.tell() < total_bytes:
                head += f.read(1).decode('utf-8')
                if len(head) > 1 and head[-1:] == '\n':
                    head_lines_read += 1

            # Keep the end of the head position so that we don't read
            # backwards past it for the tail
            head_pos = f.tell()

            # Seek to the end and start reading ending lines
            f.seek(0, os.SEEK_END)

            # Keep reading the ending lines until we've reached the max
            # number of lines we want or have reached the head output
            tail_lines_read = 0
            tail = []
            while tail_lines_read < end_lines and f.tell() > head_pos:
                # Read each character in the line until we reach
                # the beginning or a new line
                line = []
                while f.tell() > 1:
                    f.seek(-2, os.SEEK_CUR)
                    char = f.read(1).decode('utf-8')
                    if char == '\n' or f.tell() == 0:
                        break
                    line.append(char)

                # Append the new read line
                line.reverse()
                tail.append(''.join(line))
                tail_lines_read += 1

            # Whether or not we have truncated output
            # (have hit the location of the head output)
            truncated = f.tell() != head_pos

        # Form the combined output
        output = head
        if truncated:
            output += util.outputHeader('OUTPUT TRIMMED')
        if tail:
            tail.reverse()
            output += '\n'.join(tail)

        return output
