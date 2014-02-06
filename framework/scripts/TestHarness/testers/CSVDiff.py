from RunApp import RunApp
#from options import *
from CSVDiffer import CSVDiffer

class CSVDiff(RunApp):

  def getValidParams():
    params = RunApp.getValidParams()
    params.addParam('csvdiff',           [], "A list of files to run CSVDiff on.")
    params.addParam('gold_dir',      'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/)")
    params.addParam('abs_zero',       1e-10, "Absolute zero cutoff used in exodiff comparisons.")
    params.addParam('rel_err',       5.5e-6, "Relative error value used in exodiff comparisons.")
    params.addParam('delete_output_before_running',  True, "Delete pre-existing output files before running test. Only set to False if you know what you're doing!")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, name, params):
    RunApp.__init__(self, name, params)

  def prepare(self):
    if self.specs['delete_output_before_running'] == True:
      for file in self.specs['csvdiff']:
        try:
          os.remove(os.path.join(self.specs['test_dir'], file))
        except:
          pass

  def processResults(self, moose_dir, retcode, options, output):
    (reason, output) = RunApp.processResults(self, moose_dir, retcode, options, output)

    specs = self.specs
    if reason != '' or specs['skip_checks']:
      return (reason, output)

    # Don't Run CSVDiff on Scaled Tests
    if options.scaling and specs['scale_refine']:
      return (reason, output)

    if len(specs['csvdiff']) > 0:
      differ = CSVDiffer( specs['test_dir'], specs['csvdiff'], specs['abs_zero'], specs['rel_err'] )
      msg = differ.diff()
      output += 'Running CSVDiffer.py\n' + msg
      if msg != '':
        reason = 'CSVDIFF'

    return (reason, output)
