from Tester import Tester

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

    def processResults(self, moose_dir, options, output):
        if self.exit_code != 0 :
            self.setStatus('CODE %d' % self.exit_code, self.bucket_fail)
        else:
            self.setStatus(self.exit_code, self.bucket_success)
        return output
