from RunApp import RunApp
from util import runCommand
import os

class CheckFiles(RunApp):

  def getValidParams():
    params = RunApp.getValidParams()
    params.addParam('check_files', [], "A list of files that MUST exist.")
    params.addParam('check_not_exists', [], "A list of files that must NOT exist.")
    params.addParam('delete_output_before_running',  True, "Delete pre-existing output files before running test. Only set to False if you know what you're doing!")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, name, params):
    RunApp.__init__(self, name, params)

  def prepare(self):
    if self.specs['delete_output_before_running'] == True:
      for file in self.specs['check_files'] + self.specs['check_not_exists']:
        try:
          os.remove(os.path.join(self.specs['test_dir'], file))
        except:
          pass

  def processResults(self, moose_dir, retcode, options, output):
    (reason, output) = RunApp.processResults(self, moose_dir, retcode, options, output)

    specs = self.specs
    if reason != '' or specs['skip_checks']:
      return (reason, output)

    if reason == '':
     # if still no errors, check other files (just for existence)
     for file in self.specs['check_files']:
       if not os.path.isfile(os.path.join(self.specs['test_dir'], file)):
         reason = 'MISSING FILES'
         break
     for file in self.specs['check_not_exists']:
       if os.path.isfile(os.path.join(self.specs['test_dir'], file)):
         reason = 'UNEXPECTED FILES'
         break

    return (reason, output)
