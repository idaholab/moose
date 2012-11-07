from RunApp import RunApp
from options import *
from CSVDiffer import CSVDiffer

class CSVDiff(RunApp):

  def getValidParams():
    params = RunApp.getValidParams()
    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, klass, specs):
    RunApp.__init__(self, klass, specs)

  def prepare(self):
    # Note: We cannot automatically blow away files if this test contains prereqs.  This generally implies
    #       that we are using the output of one test as the input of another.
    if self.specs[PREREQ] != None:
      for file in self.specs[CSVDIFF]:
        try:
          os.remove(os.path.join(self.specs[TEST_DIR], file))
        except:
          pass

  def processResults(self, moose_dir, retcode, options, output):
    (reason, output) = RunApp.processResults(self, moose_dir, retcode, options, output)
    if reason != '':
      return reason

    specs = self.specs

    if len(specs[CSVDIFF]) > 0:
      differ = CSVDiffer( specs[TEST_DIR], specs[CSVDIFF], specs[ABS_ZERO], specs[REL_ERR] )
      msg = differ.diff()
      output += 'Running CSVDiffer.py\n' + msg
      if msg != '':
        reason = 'CSVDIFF'

    return (reason, output)
