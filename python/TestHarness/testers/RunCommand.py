import re, os, sys, time
from Tester import Tester
from RunParallel import RunParallel # For TIMEOUT value

class RunCommand(Tester):

    @staticmethod
    def validParams():
        params = Tester.validParams()
        params.addRequiredParam('command',      "The command line to execute for this test.")
        params.addParam('test_name',          "The name of the test - populated automatically")
        return params

    def __init__(self, name, params):
        Tester.__init__(self, name, params)
        self.command = params['command']

    def getCommand(self, options):
        # Create the command line string to run
        return self.command

    def processResults(self, moose_dir, retcode, options, output):
        if retcode != 0 :
            self.setStatus('CODE %d' % retcode, self.bucket_fail)
        else:
            self.setStatus(retcode, self.bucket_success)
        return output
