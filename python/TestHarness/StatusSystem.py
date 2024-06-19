#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from collections import namedtuple
import threading
import contextlib

def initStatus():
    status = namedtuple('status', 'status color code sort_value')
    return status

class StatusSystemError(Exception):
    pass

class StatusSystem(object):
    """
    A Class for supplying statuses, with status text color and corresponding exit codes.

    Set locking=True within the initializer to enable thread-safe access.

    Syntax:
    status = StatusSystem()

    status.getStatus()
      returns a named tuple:
      status(status='NA', color='GREY', code=0x0, sort_value=0)

    status.setStatus(status.fail)
      returns a named tuple:
      status(status='FAIL', color='RED', code=0x80, sort_value=390)


    Available statuses:

      no_status   the default status when instanced
      success     exit code 0, passing
      skip        exit code 0, skipped
      silent      exit code 0, skipped and also silent
      fail        exit code 0x80, a general error
      diff        exit code 0x81, an error due to exodiff, csvdiff
      deleted     exit code 0x83, some tests are instructed not to run (never fixable), but when instructed to do so they fail
      error       exit code 0x84, an error caused by the TestHarness itself
      race        exit code 0x85, an error for when race conditions are detected
      timeout     exit code 0x1, an error caused by a test exceeding it's max_time
      hold        exit code 0, used to identify the state of a test as it moves around in the TestHarness
      queued      exit code 0, used to identify the state of a test as it moves around in the TestHarness
      running     exit code 0, used to identify the state of a test as it moves around in the TestHarness
      finished    exit code 0, used to identify the state of a test as it moves around in the TestHarness
    """
    status = initStatus()

    # Default statuses
    no_status = status(status='NA', color='GREY', code=0x0, sort_value=0)

    # exit-zero statuses
    success = status(status='OK', color='GREEN', code=0x0, sort_value=100)
    skip = status(status='SKIP', color='GREY', code=0x0, sort_value=120)
    silent = status(status='SILENT', color='GREY', code=0x0, sort_value=110)

    # non-zero statuses
    fail = status(status='FAIL', color='RED', code=0x80, sort_value=390)
    diff = status(status='DIFF', color='YELLOW', code=0x81, sort_value=350)
    deleted = status(status='DELETED', color='RED', code=0x83, sort_value=300)
    error  = status(status='ERROR', color='RED', code=0x84, sort_value=360)
    race = status(status='RACE', color='RED', code=0x85, sort_value=370)
    timeout  = status(status='TIMEOUT', color='RED', code=0x1, sort_value=380)

    # Pending statuses
    hold  = status(status='HOLD', color='CYAN', code=0x0, sort_value=210)
    queued  = status(status='QUEUED', color='CYAN', code=0x0, sort_value=220)
    running  = status(status='RUNNING', color='CYAN', code=0x0, sort_value=230)

    # all-encompassing finished status
    finished  = status(status='FINISHED', color='GREY', code=0x0, sort_value=0)

    __all_statuses = [no_status,
                      success,
                      skip,
                      silent,
                      fail,
                      diff,
                      deleted,
                      error,
                      timeout,
                      hold,
                      queued,
                      running,
                      finished]

    __exit_nonzero_statuses = [fail,
                               diff,
                               deleted,
                               error,
                               timeout]

    __exit_zero_statuses = [success,
                            skip,
                            silent]

    __pending_statuses = [hold,
                          queued,
                          running]

    def __init__(self, locking=False):
        # The underlying status
        self.__status = self.no_status
        # The lock for reading/changing the status, if any
        if locking:
            self.__lock = threading.Lock()
        else:
            self.__lock = None

    def getLock(self):
        """
        Gets the thread lock for this system, if any.

        This is safe to use in a with statement even if locking
        is not enabled.
        """
        return self.__lock if self.__lock else contextlib.suppress()

    def createStatus(self, status_key='NA'):
        """ return a specific status object based on supplied status name """
        for status in self.__all_statuses:
            if status_key == status.status:
                return status

    def getStatus(self):
        """
        Return the status object.

        This is thread-safe if initialized with locking=True.
        """
        with self.getLock():
            return self.__status

    @staticmethod
    def getAllStatuses():
        """ return list of named tuples containing all status types """
        return StatusSystem.__all_statuses

    @staticmethod
    def getFailingStatuses():
        """ return list of named tuples containing failing status types """
        return StatusSystem.__exit_nonzero_statuses

    @staticmethod
    def getSuccessStatuses():
        """ return list of named tuples containing exit code zero status types """
        return StatusSystem.__exit_zero_statuses

    @staticmethod
    def getPendingStatuses():
        """ return list of named tuples containing pending status types """
        return StatusSystem.__pending_statuses

    def setStatus(self, status=no_status):
        """
        Set the current status to status. If status is not supplied, 'no_status' is implied.
        There is a validation check during this process to ensure the named tuple adheres to
        this class's set statuses.

        This is thread-safe if initialized with locking=True.
        """
        with self.getLock():
            if self.isValid(status):
                self.__status = status
            else:
                raise StatusSystemError('Invalid status! %s' % (str(status)))
            return self.__status

    @staticmethod
    def isValid(status):
        original = set(StatusSystem.no_status._asdict().keys())
        altered = set(status._asdict().keys())
        if not original.difference(altered) or status in StatusSystem.getAllStatuses():
            return True
