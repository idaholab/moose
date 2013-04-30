from InputParameters import InputParameters
import os, sys

if os.environ.has_key("MOOSE_DIR"):
  MOOSE_DIR = os.environ['MOOSE_DIR']
else:
  MOOSE_DIR = os.path.abspath(os.path.dirname(sys.argv[0])) + '/..'

class Job(object):
  def getValidParams():
    params = InputParameters()
    params.addRequiredParam('type', "The type of test of Tester to create for this test.")
    params.addParam('template_script', MOOSE_DIR + '/scripts/ClusterLauncher/pbs_submit.sh', "The template job script to use.")
    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, name, params):
    self.specs = params

  def prepareJobScript(self):
    return

  def launch(self):
    return
