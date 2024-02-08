#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from RunApp import RunApp
import os, signal,time


# Classes that derive from this class are expected to write
# output files. The Tester::getOutputFiles() method should
# be implemented for all derived classes.
class SignalTester(RunApp):
    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addParam('signal', "SIGUSR1", "The signal to send to the app. Defaults to SIGUSR1")
        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

        valid_signals = {}
        for sig in signal.valid_signals():
            try:
                valid_signals[sig.name] = sig
            # Skip int signals that don't have names.
            except AttributeError:
                continue
        try:
            self.signal = valid_signals[self.specs["signal"]]
        except KeyError as e:
            print(f"Error with parameter 'signal': {self.specs['signal']} is not "
                  f"a supported signal type. Currently supported signal types are:\n{', '.join(list(valid_signals.keys()))}")
            raise e

    def send_signal(self):
        """Function used to send a signal to the program automatically for testing purposes."""

        # Create a while loop that checks if the stdout buffer has any data in it, and then sends the signal once
        # it knows that the moose_test binary is actually doing something.

        # process.poll() returns the process's exit code if it has completed, and None if it is still running.
        # This acts as a safety precaution against an infinite loop -- this will always close.
        while self.process.poll() is None:

            # tell() gives the current position in the file. If it is greater than zero, the binary
            # has started running and writing output.
            # if the output is blank, the moose_test binary hasn't actually started doing anything yet.
            # if so, sleep briefly and check again.
            if not self.outfile.tell():
                time.sleep(0.05)

            # if the output isn't blank, then we finally send the signal and exit the loop
            else:
                try:
                    os.kill(self.process.pid, self.signal)
                    break
                except ProcessLookupError as e:
                    print("Unable to send signal to process. Has it already terminated?")
                    raise e

    def runCommand(self, timer, options):
        """
        Helper method for running external (sub)processes as part of the tester's execution.  This
        uses the tester's getCommand and getTestDir methods to run a subprocess.  The timer must
        be the same timer passed to the run method.  Results from running the subprocess is stored
        in the tester's output and exit_code fields.
        """

        exit_code = super().spawnSubprocessFromOptions(timer, options)
        if exit_code: # Something went wrong
            return

        self.send_signal()
        super().finishAndCleanupSubprocess(timer)
