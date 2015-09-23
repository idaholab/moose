from util import *
from Tester import Tester
import os, subprocess

class CrowPython(Tester):
  """ A python test interface for Crow """
  try:
    output_swig = subprocess.Popen(["swig","-version"],stdout=subprocess.PIPE).communicate()[0]
  except OSError:
    output_swig = "Failed"

  has_swig2 = "Version 2.0" in output_swig or "Version 3.0" in output_swig

  @staticmethod
  def validParams():
  """ Return a list of valid parameters and their descriptions for this type of
      test. """
    params = Tester.validParams()
    params.addRequiredParam('input',"The python file to use for this test.")
    if os.environ.get("CHECK_PYTHON3","0") == "1":
      params.addParam('python_command','python3','The command to use to run python')
    else:
      params.addParam('python_command','python','The command to use to run python')
    params.addParam('requires_swig2', False, "Requires swig2 for test")
    return params

  def getCommand(self, options):
    """ Return the command this test will run. """
    return self.specs["python_command"]+" "+self.specs["input"]

  def __init__(self, name, params):
    """ Constructor that will setup this test with a name and a list of
        parameters.
        @ In, name: the name of this test case.
        @ In, params, a dictionary of parameters and their values to use.
    """
    Tester.__init__(self, name, params)
    self.specs['scale_refine'] = False

  def __init__(self, name, params):
    """ Constructor that will setup this test with a name and a list of
        parameters.
        @ In, name: the name of this test case.
        @ In, params, a dictionary of parameters and their values to use.
    """
    Tester.__init__(self, name, params)
    self.specs['scale_refine'] = False

  def checkRunnable(self, option):
    """ Checks if a test case is capable of being run on the current system. """
    if self.specs['requires_swig2'] and not CrowPython.has_swig2:
      return (False, 'skipped (No swig 2.0 found)')
    return (True, '')

  def processResults(self, moose_dir,retcode, options, output):
    """ Handle the results of test
        @ In, moose_dir: the root directory where MOOSE resides on the current
                         system.
        @ In, retcode: Return code of the test case.
        @ In, options: options (unused)
        @ In, output: the output from the test case.
        @ Out: a tuple with the error return code and the output passed in.
    """
    if retcode != 0:
      return (str(retcode),output)
    return ('',output)