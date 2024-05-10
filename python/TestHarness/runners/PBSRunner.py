#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarness.runners.Runner import Runner
import re, time, os, subprocess

class PBSRunner(Runner):
    """Runner that spawns a process with PBS.

    To be used with the RunPBS scheduler.
    """
    def __init__(self, job, options, run_pbs):
        Runner.__init__(self, job, options)
        self.run_pbs = run_pbs

        # Number of seconds to try to wait for the output
        # We don't want to wait forever for output because
        # if the job ended in an unexpected state, it might
        # not even be using the output and we don't want to
        # just hang forever
        self.wait_output_time = 120

    def spawn(self, timer):
        from TestHarness.schedulers.RunPBS import RunPBS

        # Submit the job
        self.run_pbs.submitJob(self.job)

        timer.start()

    def wait(self, timer):
        # Need to import here to avoid cyclic includes
        from TestHarness.schedulers.RunPBS import RunPBS

        # Poll loop waiting for the job to be finished
        # This gets a structure that represents the job, and the
        # polling itself is only done on occasion within RunPBS
        while True:
            time.sleep(1)
            pbs_job = self.run_pbs.getPBSJob(self.job)

            # We're done
            if pbs_job.done:
                self.exit_code = pbs_job.exit_code
                break

        timer.stop()

        # The PBS output (stdout+stderr)
        output_file = self.run_pbs.getPBSJobOutputPath(self.job)

        # If the Job is already finished, something happened in PBS
        # so we have an invalid state for processing in the Tester
        if self.job.isFinished():
            self.exit_code = -1
            self.output = ''

            # If we have output, we should try to add it
            # TODO: shorten output as an option?
            if os.path.exists(output_file) and os.path.isfile(output_file):
                try:
                    self.output = open(file, 'r').read()
                except:
                    pass

            # Don't bother looking for the rest of the output
            return

        tester = self.job.getTester()

        # We've actually ran something now and not just qsub, so update the
        # command to what was ran there
        tester.setCommandRan(pbs_job.command)

        # Determine the output files that we need to wait for to be complete
        wait_files = set([output_file])
        # Output files needed by the Tester, only if it says we should
        if tester.mustOutputExist():
            for file in tester.getOutputFiles(self.options):
                wait_files.add(os.path.join(tester.getTestDir(), file))
        # The files that we can read, but are incomplete (no terminator)
        incomplete_files = set()

        # Wait for all of the files to be available
        file_poll_interval = 0.25
        waited_time = 0
        while wait_files or incomplete_files:
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
                        self.output = open(file, 'r').read()
                    # Done with this file
                    incomplete_files.discard(file)

            # We've waited for files for too long
            if (wait_files or incomplete_files) and waited_time >= self.wait_output_time:
                self.job.setStatus(self.job.error, 'FILE TIMEOUT')
                if not self.output:
                    self.output = ''
                def print_files(files, type):
                    if files:
                        self.output += '#' * 80 + f'\n{type} output file(s)\n' + '#' * 80 + '\n'
                        for file in files:
                            self.output += file + '\n'
                        self.output += '\n'
                print_files(wait_files, 'Unavailable')
                print_files(incomplete_files, 'Incomplete')
                break

            waited_time += file_poll_interval
            time.sleep(file_poll_interval)

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

        ending_comment = self.run_pbs.getOutputEndingComment()

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
    def removeLastLine(file):
        """
        Removes the last line from the given text file.

        Used to remove the terminator that we append to all output
        files on the compute host in order to make sure that the
        entire output file is synced"""
        # stackoverflow.com/questions/1877999/delete-final-line-in-file-with-python
        with open(file, "r+", encoding="utf-8") as f:
            f.seek(0, os.SEEK_END)
            pos = f.tell() - 1
            while pos > 0 and f.read(1) != "\n":
                pos -= 1
                f.seek(pos, os.SEEK_SET)
            if pos > 0:
                f.seek(pos, os.SEEK_SET)
                f.truncate()

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

    def kill(self):
        self.run_pbs.killJob(self.job)
