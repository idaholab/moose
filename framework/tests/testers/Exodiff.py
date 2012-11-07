from RunApp import RunApp
from options import *
from util import runCommand
import os

class Exodiff(RunApp):

  def getValidParams():
    params = RunApp.getValidParams()
    params.addParam('exodiff',               "A list of files to exodiff.")
    params.addParam('exodiff_opts',          "Additional arguments to be passed to invocations of exodiff.")
    params.addParam('gold_dir',     'gold',  "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/)")
    params.addParam('abs_zero',      1e-10,  "Absolute zero cutoff used in exodiff comparisons.")
    params.addParam('rel_err',      5.5e-6,  "Relative error value used in exodiff comparisons.")
    params.addParam('custom_cmp',            "Custom comparison file")
    params.addParam('use_old_floor', False,  "Use Exodiff old floor option")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, klass, specs):
    RunApp.__init__(self, klass, specs)

  def prepare(self):
    # Note: We cannot automatically blow away files if this test contains prereqs.  This generally implies
    #       that we are using the output of one test as the input of another.
    if self.specs[PREREQ] != None:
      for file in self.specs[EXODIFF]:
        try:
          os.remove(os.path.join(specs[TEST_DIR], file))
        except:
          pass

  def processResults(self, moose_dir, retcode, options, output):
    (reason, output) = RunApp.processResults(self, moose_dir, retcode, options, output)
    if reason != '':
      return reason

    specs = self.specs

    for file in specs[EXODIFF]:
      custom_cmp = ''
      old_floor = ''
      if specs[CUSTOM_CMP] != None:
         custom_cmp = ' -f ' + os.path.join(specs[TEST_DIR], specs[CUSTOM_CMP])
      if specs[USE_OLD_FLOOR]:
         old_floor = ' -use_old_floor'

#      # see if the output file has been written (keep trying...)
#      file_found = False
#      for i in xrange(0, 10):
#        if os.path.exists(os.path.join(specs[TEST_DIR], file)):
#          file_found = True
#          break
#        else:
#          sleep(0.5)

#      if file_found:

      if not os.path.exists(os.path.join(specs[TEST_DIR], specs[GOLD_DIR], file)):
        output += "File Not Found: " + os.path.join(specs[TEST_DIR], specs[GOLD_DIR], file)
        reason = 'MISSING GOLD FILE'
        break
      else:
        command = moose_dir + 'contrib/exodiff/exodiff -m' + custom_cmp + ' -F' + ' ' + str(specs[ABS_ZERO]) + old_floor + ' -t ' + str(specs[REL_ERR]) \
            + ' ' + ' '.join(specs[EXODIFF_OPTS]) + ' ' + os.path.join(specs[TEST_DIR], specs[GOLD_DIR], file) + ' ' + os.path.join(specs[TEST_DIR], file)
        exo_output = runCommand(command)

        output += 'Running exodiff: ' + command + '\n' + exo_output + ' ' + ' '.join(specs[EXODIFF_OPTS])

        if ('different' in exo_output or 'ERROR' in exo_output) and not "Files are the same" in exo_output:
          reason = 'EXODIFF'
          break
#      else:
#        reason = 'NO EXODIFF FILE'
#        break

    return (reason, output)
