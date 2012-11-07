from RunApp import RunApp
from options import *
from util import runCommand
import os

class CheckFiles(RunApp):

  def getValidParams():
    params = RunApp.getValidParams()
    params.addParam('check_files', "A list of files to exodiff.")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, klass, specs):
    RunApp.__init__(self, klass, specs)

  def prepare(self):
    # Note: We are going to blow away all files listed in 'check_files'.  Prereqs SHOULD NOT rely on these files existing.
    for file in self.specs[CHECK_FILES]:
      try:
        os.remove(os.path.join(self.specs[TEST_DIR], file))
      except:
        pass

  def processResults(self, moose_dir, retcode, options, output):
    reason = RunApp.processResults(self, moose_dir, retcode, options, output)

    # if still no errors, check other files (just for existence)
    if reason == '':
      for file in self.specs[CHECK_FILES]:
        if not os.path.isfile(os.path.join(self.specs[TEST_DIR], file)):
          reason = 'MISSING FILES'
          break

    return reason
