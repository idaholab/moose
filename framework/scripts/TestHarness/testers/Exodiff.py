from RunApp import RunApp
#from options import *
from util import runCommand
import os

class Exodiff(RunApp):

  def getValidParams():
    params = RunApp.getValidParams()
    params.addRequiredParam('exodiff',   [], "A list of files to exodiff.")
    params.addParam('exodiff_opts',      [], "Additional arguments to be passed to invocations of exodiff.")
    params.addParam('gold_dir',      'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/)")
    params.addParam('abs_zero',       1e-10, "Absolute zero cutoff used in exodiff comparisons.")
    params.addParam('rel_err',       5.5e-6, "Relative error value used in exodiff comparisons.")
    params.addParam('custom_cmp',            "Custom comparison file")
    params.addParam('use_old_floor',  False, "Use Exodiff old floor option")
    params.addParam('delete_output_before_running',  True, "Delete pre-existing output files before running test. Only set to False if you know what you're doing!")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, name, params):
    RunApp.__init__(self, name, params)

  def prepare(self):
    if self.specs['delete_output_before_running'] == True:
      for file in self.specs['exodiff']:
        try:
          os.remove(os.path.join(self.specs['test_dir'], file))
        except:
          pass

  def processResults(self, moose_dir, retcode, options, output):
    (reason, output) = RunApp.processResults(self, moose_dir, retcode, options, output)

    specs = self.specs
    if reason != '' or specs['skip_checks']:
      return (reason, output)

    # Don't Run Exodiff on Scaled Tests
    if options.scaling and specs['scale_refine']:
      return (reason, output)

    for file in specs['exodiff']:
      custom_cmp = ''
      old_floor = ''
      if specs.isValid('custom_cmp'):
         custom_cmp = ' -f ' + os.path.join(specs['test_dir'], specs['custom_cmp'])
      if specs['use_old_floor']:
         old_floor = ' -use_old_floor'

      if not os.path.exists(os.path.join(specs['test_dir'], specs['gold_dir'], file)):
        output += "File Not Found: " + os.path.join(specs['test_dir'], specs['gold_dir'], file)
        reason = 'MISSING GOLD FILE'
        break
      else:
        command = moose_dir + 'contrib/exodiff/exodiff -m' + custom_cmp + ' -F' + ' ' + str(specs['abs_zero']) + old_floor + ' -t ' + str(specs['rel_err']) \
            + ' ' + ' '.join(specs['exodiff_opts']) + ' ' + os.path.join(specs['test_dir'], specs['gold_dir'], file) + ' ' + os.path.join(specs['test_dir'], file)
        exo_output = runCommand(command)

        output += 'Running exodiff: ' + command + '\n' + exo_output + ' ' + ' '.join(specs['exodiff_opts'])

        if ('different' in exo_output or 'ERROR' in exo_output) and not "Files are the same" in exo_output:
          reason = 'EXODIFF'
          break

    return (reason, output)
