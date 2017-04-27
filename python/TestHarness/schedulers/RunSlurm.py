from MooseObject import MooseObject
from Scheduler import Scheduler
from QueueManager import QueueManager

## This class launches jobs using the PBS queuing system
class RunSlurm(QueueManager):
    @staticmethod
    def validParams():
        params = QueueManager.validParams()
        params.addRequiredParam('scheduler',       'RunSlurm', "the name of the scheduler used")

        return params

    ## Return this return code if the process must be killed because of timeout
    TIMEOUT = -999999

    def __init__(self, harness, params):
        QueueManager.__init__(self, harness, params)

    ## run the command asynchronously and call testharness.testOutputAndFinish when complete
    def run(self, tester, command, recurse=True, slot_check=True):
        return

    ## Return control to the test harness by finalizing the test output and calling the callback
    def returnToTestHarness(self, job_index):
        return
