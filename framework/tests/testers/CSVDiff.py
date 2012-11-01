from RunApp import RunApp
from options import *
from CSVDiffer import CSVDiffer

class CSVDiff(RunApp):
  def __init__(self, klass, specs):
    RunApp.__init__(self, klass, specs)

  def prepare(self):
    return

  def processResults(self, moose_dir, retcode, output):
    specs = self.specs
    reason = ''

    if len(test[CSVDIFF]) > 0:
      differ = CSVDiffer( specs[TEST_DIR], specs[CSVDIFF], specs[ABS_ZERO], specs[REL_ERR] )
      msg = differ.diff()
      output += 'Running CSVDiffer.py\n' + msg
      if msg != '':
        reason = 'CSVDIFF'

    return reason
