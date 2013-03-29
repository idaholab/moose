from RunApp import RunApp
from options import *
from CSVDiffer import CSVDiffer

class CSVDiff(RunApp):

  def getValidParams():
    params = RunApp.getValidParams()
    params.addParam('csvdiff',           [], "A list of files to run CSVDiff on.")
    params.addParam('gold_dir',      'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/)")
    params.addParam('abs_zero',       1e-10, "Absolute zero cutoff used in exodiff comparisons.")
    params.addParam('rel_err',       5.5e-6, "Relative error value used in exodiff comparisons.")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, name, params):
    RunApp.__init__(self, name, params)

  def prepare(self):
    for file in self.specs[CSVDIFF]:
      try:
        os.remove(os.path.join(self.specs[TEST_DIR], file))
      except:
        pass

  def processResults(self, moose_dir, retcode, options, output):
    (reason, output) = RunApp.processResults(self, moose_dir, retcode, options, output)
    if reason != '':
      return (reason, output)

    specs = self.specs

    # Don't Run CSVDiff on Scaled Tests
    if options.scaling and specs[SCALE_REFINE]:
      return (reason, output)

    if len(specs[CSVDIFF]) > 0:
      differ = CSVDiffer( specs[TEST_DIR], specs[CSVDIFF], specs[ABS_ZERO], specs[REL_ERR] )
      msg = differ.diff()
      output += 'Running CSVDiffer.py\n' + msg
      if msg != '':
        reason = 'CSVDIFF'

    return (reason, output)
