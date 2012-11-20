from options import *
from InputParameters import InputParameters

class Tester(object):
  # Static Method
  def getValidParams():
    params = InputParameters()

    # Common Options
    params.addRequiredParam('type', "The type of test of Tester to create for this test.")
    params.addParam('max_time',   300, "The maximum in seconds that the test will be allowed to run.")
    params.addParam('skip',     False, "If supplied will skip the test and print the reason given for doing so.")
    params.addParam('deleted',         "Tests that only show up when using the '-e' option (Permanently skipped or not implemented).")

    params.addParam('heavy',    False, "Set to True if this test should only be run when the '--heavy' option is used.")
    params.addParam('group',       [], "A list of groups for which this test belongs.")
    params.addParam('prereq',      [], "A list of prereq tests that need to run successfully before launching this test.")

    # Test Filters
    params.addParam('platform',      ['ALL'], "A list of platforms for which this test will run on. ('ALL', 'DARWIN', 'LINUX', 'SL', 'LION', 'ML')")
    params.addParam('compiler',      ['ALL'], "A list of compilers for which this test is valid on. ('ALL', 'GCC', 'INTEL', 'CLANG')")
    params.addParam('petsc_version', ['ALL'], "A list of petsc versions for which this test will run on, supports normal comparison operators ('<', '>', etc...)")
    params.addParam('mesh_mode',     ['ALL'], "A list of mesh modes for which this test will run ('PARALLEL_MESH', 'SERIAL_MESH')")
    params.addParam('method',        ['ALL'], "A test that runs under certain executable configurations ('ALL', 'OPT', 'DBG')")
    params.addParam('library_mode',  ['ALL'], "A test that only runs when libraries are built under certain configurations ('ALL', 'STATIC', 'DYNAMIC')")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, name, params):
    self.specs = params

  def prepare(self):
    return

  def getCommand(self, options):
    return

  def processResults(self, moose_dir, retcode, options, output):
    return

