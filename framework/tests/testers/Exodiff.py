from RunApp import RunApp
from options import *
from util import runCommand
import os

class Exodiff(RunApp):
  def __init__(self, klass, specs):
    RunApp.__init__(self, klass, specs)

  def prepare(self):
    return

  def processResults(self, moose_dir, retcode, output):
    specs = self.specs
    reason = ''

    for file in specs[EXODIFF]:
      custom_cmp = ''
      old_floor = ''
      if specs[CUSTOM_CMP] != None:
         custom_cmp = ' -f ' + os.path.join(specs[TEST_DIR], specs[CUSTOM_CMP])
      if specs[USE_OLD_FLOOR]:
         old_floor = ' -use_old_floor'

      # see if the output file has been written (keep trying...)
      file_found = False
      for i in xrange(0, 10):
        if os.path.exists(os.path.join(specs[TEST_DIR], file)):
          file_found = True
          break
        else:
          sleep(0.5)

      if file_found:
        command = moose_dir + 'contrib/exodiff/exodiff -m' + custom_cmp + ' -F' + ' ' + str(specs[ABS_ZERO]) + old_floor + ' -t ' + str(specs[REL_ERR]) \
            + ' ' + ' '.join(specs[EXODIFF_OPTS]) + ' ' + os.path.join(specs[TEST_DIR], specs[GOLD_DIR], file) + ' ' + os.path.join(specs[TEST_DIR], file)
        exo_output = runCommand(command)

        output += 'Running exodiff: ' + command + '\n' + exo_output + ' ' + ' '.join(specs[EXODIFF_OPTS])

        if ('different' in exo_output or 'ERROR' in exo_output) and not "Files are the same" in exo_output:
          reason = 'EXODIFF'
          break
      else:
        reason = 'NO EXODIFF FILE'
        break
    return reason
