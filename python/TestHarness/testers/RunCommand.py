#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from Tester import Tester

class RunCommand(Tester):

    @staticmethod
    def validParams():
        params = Tester.validParams()
        params.addRequiredParam('command',      "The command line to execute for this test.")
        params.addParam('test_name',          "The name of the test - populated automatically")
        params.addParam('job_slots',    1, "Inform the TestHarness how many cores this job is"
                                           " expected to consume")
        return params

    def __init__(self, name, params):
        Tester.__init__(self, name, params)
        self.command = params['command']

    def getSlots(self, options) ->int:
        """ return param job slots, default to 1 if 0 is provided """
        # insure zero can't be used
        return max(1, int(self.specs['job_slots']))

    def getCommand(self, options):
        # Create the command line string to run
        return self.command

    def processResults(self, moose_dir, options, output):
        if self.exit_code == 77 :
            self.setStatus(self.skip)
        elif self.exit_code != 0 :
            self.setStatus(self.fail, 'CODE %d' % self.exit_code)

        return output
