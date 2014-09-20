import re, os, sys
from Tester import Tester
from RunParallel import RunParallel # For TIMEOUT value

class PeacockTester(Tester):

  @staticmethod
  def validParams():
    params = Tester.validParams()
    params.addRequiredParam('function', "The test function to execute")
    params.addParam('test_name', "The name of the test - populated automatically")
    return params

  def __init__(self, name, params):
    Tester.__init__(self, name, params)

  def checkRunnable(self, options):
    return (True, '')

  def getCommand(self, options):
    return ''

  def processResults(self, moose_dir, retcode, options, output):
    return (True, output)
