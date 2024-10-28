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

    def checkRunnable(self, options):
        # We could probably configure sending signals via pbs and slurm
        # but for now that's a no
        if options.hpc:
            self.addCaveats('hpc unsupported')
            self.setStatus(self.skip)
            return False

        return super().checkRunnable(options)

    def postSpawn(self, runner):
        runner.sendSignal(self.signal)
