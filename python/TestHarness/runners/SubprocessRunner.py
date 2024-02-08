import os
import platform
import subprocess
import shlex
import traceback
from tempfile import SpooledTemporaryFile
from signal import SIGTERM
from TestHarness.runners.Runner import Runner
from TestHarness import util

class SubprocessRunner(Runner):
    """
    Runner that spawns a local subprocess.
    """
    def __init__(self, tester):
        Runner.__init__(self, tester)

        # The output file handler
        self.outfile = None
        # The error file handler
        self.errfile = None
        # The underlying subprocess
        self.process = None

    def spawn(self, cmd, cwd, timer):
        use_shell = self.tester.specs["use_shell"]

        # Split command into list of args to be passed to Popen
        if not use_shell:
            cmd = shlex.split(cmd)

        self.process = None
        try:
            self.outfile = SpooledTemporaryFile(max_size=1000000) # 1M character buffer
            self.errfile = SpooledTemporaryFile(max_size=100000)  # 100K character buffer

            process_args = [cmd]
            process_kwargs = {'stdout': self.outfile,
                              'stderr': self.errfile,
                              'close_fds': False,
                              'shell': use_shell,
                              'cwd': cwd}
            # On Windows, there is an issue with path translation when the command is passed in
            # as a list.
            if platform.system() == "Windows":
                process_kwargs['creationflags'] = subprocess.CREATE_NEW_PROCESS_GROUP
            else:
                process_kwargs['preexec_fn'] = os.setsid

            # Special logic for openmpi runs
            if self.hasOpenMPI():
                popen_env = os.environ.copy()

                # Don't clobber state
                popen_env['OMPI_MCA_orte_tmpdir_base'] = self.getTempDirectory().name
                # Allow oversubscription for hosts that don't have a hostfile
                popen_env['PRTE_MCA_rmaps_default_mapping_policy'] = ':oversubscribe'

                popen_kwargs['env'] = popen_env

            self.process = subprocess.Popen(*process_args, **process_kwargs)
        except:
            print("Error in launching a new task", cmd)
            traceback.print_exc()
            raise

        timer.start()

    def wait(self, timer):
        self.process.wait()

        timer.stop()

        self.exit_code = self.process.poll()
        self.outfile.flush()
        self.errfile.flush()

        # store the contents of output, and close the file
        self.outfile.close()
        self.errfile.close()

    def kill(self):
        if self.process is not None:
            try:
                if platform.system() == "Windows":
                    from distutils import spawn
                    if spawn.find_executable("taskkill"):
                        subprocess.call(['taskkill', '/F', '/T', '/PID', str(self.process.pid)])
                    else:
                        self.process.terminate()
                else:
                    pgid = os.getpgid(self.process.pid)
                    os.killpg(pgid, SIGTERM)
            except OSError: # Process already terminated
                pass

    def getOutput(self):
        return util.readOutput(self.outfile, self.errfile, self)

    def isOutputReady(self):
        return not self.outfile is None and self.outfile.closed and not self.errfile is None and self.errfile.closed
