from RunApp import RunApp
from util import runCommand
import os

class Exodiff(RunApp):

  @staticmethod
  def validParams():
    params = RunApp.validParams()
    params.addRequiredParam('exodiff',   [], "A list of files to exodiff.")
    params.addParam('exodiff_opts',      [], "Additional arguments to be passed to invocations of exodiff.")
    params.addParam('gold_dir',      'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/)")
    params.addParam('abs_zero',       1e-10, "Absolute zero cutoff used in exodiff comparisons.")
    params.addParam('rel_err',       5.5e-6, "Relative error value used in exodiff comparisons.")
    params.addParam('custom_cmp',            "Custom comparison file")
    params.addParam('use_old_floor',  False, "Use Exodiff old floor option")
    params.addParam('delete_output_before_running',  True, "Delete pre-existing output files before running test. Only set to False if you know what you're doing!")
    params.addParam('delete_output_folders', True, "Delete output folders before running")
    params.addParam('map',  True, "Use geometrical mapping to match up elements.  This is usually a good idea because it makes files comparable between runs with Serial and Parallel Mesh.")

    return params

  def __init__(self, name, params):
    RunApp.__init__(self, name, params)


  def prepare(self):
    if self.specs['delete_output_before_running'] == True:
      self.deleteFilesAndFolders(self.specs['test_dir'], self.specs['exodiff'], self.specs['delete_output_folders'])

  def processResultsCommand(self, moose_dir, options):
    commands = []

    for file in self.specs['exodiff']:
      custom_cmp = ''
      old_floor = ''
      if self.specs.isValid('custom_cmp'):
         custom_cmp = ' -f ' + os.path.join(self.specs['test_dir'], self.specs['custom_cmp'])
      if self.specs['use_old_floor']:
         old_floor = ' -use_old_floor'

      if self.specs['map']:
         map_option = ' -m '
      else:
         map_option = ' '

      commands.append(os.path.join(moose_dir, 'framework', 'contrib', 'exodiff', 'exodiff') + map_option + custom_cmp + ' -F' + ' ' + str(self.specs['abs_zero']) \
                      + old_floor + ' -t ' + str(self.specs['rel_err']) + ' ' + ' '.join(self.specs['exodiff_opts']) + ' ' \
                      + os.path.join(self.specs['test_dir'], self.specs['gold_dir'], file) + ' ' + os.path.join(self.specs['test_dir'], file))

    return commands


  def processResults(self, moose_dir, retcode, options, output):
    (reason, output) = RunApp.processResults(self, moose_dir, retcode, options, output)

    if reason != '' or self.specs['skip_checks']:
      return (reason, output)

    # Don't Run Exodiff on Scaled Tests
    if options.scaling and self.specs['scale_refine']:
      return (reason, output)

    # Make sure that all of the Exodiff files are actually available
    for file in self.specs['exodiff']:
      if not os.path.exists(os.path.join(self.specs['test_dir'], self.specs['gold_dir'], file)):
        output += "File Not Found: " + os.path.join(self.specs['test_dir'], self.specs['gold_dir'], file)
        reason = 'MISSING GOLD FILE'
        break

    if reason == '':
      # Retrieve the commands
      commands = self.processResultsCommand(moose_dir, options)

      for command in commands:
        exo_output = runCommand(command)

        output += 'Running exodiff: ' + command + '\n' + exo_output + ' ' + ' '.join(self.specs['exodiff_opts'])

        if ('different' in exo_output or 'ERROR' in exo_output) and not "Files are the same" in exo_output:
          reason = 'EXODIFF'
          break

    return (reason, output)
