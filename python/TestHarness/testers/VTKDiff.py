from RunApp import RunApp
from util import runCommand
import os
from XMLDiffer import XMLDiffer

class VTKDiff(RunApp):

  @staticmethod
  def validParams():
    params = RunApp.validParams()
    params.addRequiredParam('vtkdiff',   [], "A list of files to exodiff.")
    params.addParam('gold_dir',      'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/)")
    params.addParam('abs_zero',       1e-10, "Absolute zero cutoff used in exodiff comparisons.")
    params.addParam('rel_err',       5.5e-6, "Relative error value used in exodiff comparisons.")
    params.addParam('ignored_attributes',  [], "Ignore e.g. type and/or version in sample XML block <VTKFile type=\"Foo\" version=\"0.1\">")

    return params

  def __init__(self, name, params):
    RunApp.__init__(self, name, params)

  def prepare(self, options):
    if self.specs['delete_output_before_running'] == True:
      self.deleteFilesAndFolders(self.specs['test_dir'], self.specs['vtkdiff'])

  def processResults(self, moose_dir, retcode, options, output):
    output = RunApp.processResults(self, moose_dir, retcode, options, output)

    # Skip
    specs = self.specs
    if self.getStatus() == 'FAIL' or specs['skip_checks']:
      return output

    # Don't Run VTKDiff on Scaled Tests
    if options.scaling and specs['scale_refine']:
      return output

    # Loop over every file
    for file in specs['vtkdiff']:

      # Error if gold file does not exist
      if not os.path.exists(os.path.join(specs['test_dir'], specs['gold_dir'], file)):
        output += "File Not Found: " + os.path.join(specs['test_dir'], specs['gold_dir'], file)
        self.setStatus('MISSING GOLD FILE', 'FAIL')
        break

      # Perform diff
      else:
        for file in self.specs['vtkdiff']:
          gold = os.path.join(specs['test_dir'], specs['gold_dir'], file)
          test = os.path.join(specs['test_dir'], file)

          # We always ignore the header_type attribute, since it was
          # introduced in VTK 7 and doesn't seem to be important as
          # far as Paraview is concerned.
          specs['ignored_attributes'].append('header_type')

          differ = XMLDiffer(gold, test, abs_zero=specs['abs_zero'], rel_tol=specs['rel_err'], ignored_attributes=specs['ignored_attributes'])

          # Print the results of the VTKDiff whether it passed or failed.
          output += differ.message() + '\n'

          if differ.fail():
            self.setStatus('VTKDIFF', 'DIFF')
            break

    # Return to the test harness
    return output
