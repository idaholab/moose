from RunApp import RunApp
from options import *
from util import runCommand
import os

class CheckFiles(RunApp):

  def getValidParams():
    params = RunApp.getValidParams()
    params.addParam('check_files', [], "A list of files that MUST exist.")
    params.addParam('check_not_exists', [], "A list of files that must NOT exist.")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, name, params):
    RunApp.__init__(self, name, params)

  def prepare(self):
    for file in self.specs[CHECK_FILES] + self.specs[CHECK_NOT_EXISTS]:
      try:
        os.remove(os.path.join(self.specs[TEST_DIR], file))
      except:
        pass

  def processResults(self, moose_dir, retcode, options, output):
    (reason, output) = RunApp.processResults(self, moose_dir, retcode, options, output)

    if reason == '':
     # if still no errors, check other files (just for existence)
     for file in self.specs[CHECK_FILES]:
       if not os.path.isfile(os.path.join(self.specs[TEST_DIR], file)):
         reason = 'MISSING FILES'
         break
     for file in self.specs[CHECK_NOT_EXISTS]:
       if os.path.isfile(os.path.join(self.specs[TEST_DIR], file)):
         reason = 'UNEXPECTED FILES'
         break

    return (reason, output)
