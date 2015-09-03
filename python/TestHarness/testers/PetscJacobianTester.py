from RunApp import RunApp
from util import runCommand
import os, re

class PetscJacobianTester(RunApp):

  @staticmethod
  def validParams():
    params = RunApp.validParams()
    params.addParam('ratio_tol', 1e-8, "Relative tolerance to compare the ration against.")
    params.addParam('difference_tol', 1e-8, "Relative tolerance to compare the difference against.")

    return params

  def checkRunnable(self, options):
    if options.enable_recover:
      reason = 'skipped (PetscJacTester RECOVER)'
      return (False, reason)
    return RunApp.checkRunnable(self, options)

  def __init__(self, name, params):
    RunApp.__init__(self, name, params)
    self.specs['cli_args'].append('-snes_type test')

  def processResults(self, moose_dir, retcode, options, output):
    m = re.search("Norm of matrix ratio (\S+?),? difference (\S+) \(user-defined state\)", output, re.MULTILINE | re.DOTALL);
    if m:
      if float(m.group(1)) < float(self.specs['ratio_tol']) and float(m.group(2)) < float(self.specs['difference_tol']):
        reason = ''
      else:
        reason = 'INCORRECT JACOBIAN'
    else:
      reason = 'EXPECTED OUTPUT NOT FOUND';

    return (reason, output)
