import os, re
from Tester import Tester

class RunPythonApp(Tester):
  """
  Tester object for executing Python scripts.
  """

  @staticmethod
  def validParams():
    params = Tester.validParams()
    params.addRequiredParam('input', "The python input file to use for this test.")
    params.addParam('expect_out', "Search for the supplied string in the output.")
    params.addParam('match_literal', False, "Treat expect_out as a string not a regular expression.")
    params.addParam('unittest', False, "This uses unittest.")
    params.addParam('cli_args', '', "String of additional options to pass to Python executable.")
    return params

  def __init__(self, name, params):
    Tester.__init__(self, name, params)

  def getCommand(self, options):
    """
    Returns the python script to execute
    """
    return os.path.join(self.specs['test_dir'], self.specs['input']) + ' ' + self.specs['cli_args']

  # Perform image diff
  def processResults(self, moose_dir, retcode, options, output):
    """
    Checks the results from the output.
    """
    specs = self.specs

    # Initialize reason output
    reason = ''

    if specs.isValid('expect_out'):
      if specs['match_literal']:
        out_ok = self.checkOutputForLiteral(output, specs['expect_out'])
      else:
        out_ok = self.checkOutputForPattern(output, specs['expect_out'])

      # Process out_ok
      if (out_ok and retcode != 0):
        reason = 'OUT FOUND BUT CRASH'
      elif (not out_ok):
        reason = 'NO EXPECTED OUT'
    elif specs['unittest']:
      out_ok = self.checkOutputForPattern(output, "^OK$") or self.checkOutputForPattern(output, "^OK\s\(skipped=\d+\)$")
      if not out_ok:
        reason = 'FAILED'

    # Return the reason and command output
    return (reason, output)

  def checkOutputForPattern(self, output, re_pattern):
    """
    Does a pattern match for the supplied re with the output.
    """
    if re.search(re_pattern, output, re.MULTILINE | re.DOTALL) == None:
      return False
    else:
      return True

  def checkOutputForLiteral(self, output, literal):
    """
    Does a literal check for the provided string in the output.
    """
    if output.find(literal) == -1:
      return False
    else:
      return True
