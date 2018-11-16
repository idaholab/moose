#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Helper for defining custom MooseDocs logging."""
import logging
import traceback
import multiprocessing
import mooseutils

import MooseDocs

class MooseDocsFormatter(logging.Formatter):
    """
    A formatter that is aware of the class hierarchy of the MooseDocs library.
    Call the init_logging function to initialize the use of this custom formatter.
    """
    COLOR = dict(DEBUG='CYAN',
                 INFO='RESET',
                 WARNING='LIGHT_YELLOW',
                 ERROR='LIGHT_RED',
                 CRITICAL='MAGENTA')

    COUNTS = dict(CRITICAL=multiprocessing.Value('I', 0, lock=True),
                  ERROR=multiprocessing.Value('I', 0, lock=True),
                  WARNING=multiprocessing.Value('I', 0, lock=True),
                  INFO=multiprocessing.Value('I', 0, lock=True),
                  DEBUG=multiprocessing.Value('I', 0, lock=True))

    def format(self, record):
        """Format the supplied logging record and count the occurrences."""
        tid = multiprocessing.current_process().name
        msg = u'{} ({}): {}'.format(record.name, tid, logging.Formatter.format(self, record))

        with self.COUNTS[record.levelname].get_lock():
            self.COUNTS[record.levelname].value += 1

        return mooseutils.colorText(msg, self.COLOR[record.levelname])

class MultiprocessingHandler(logging.StreamHandler):
    """A simple handler that locks when writing with multiprocessing."""

    def flush(self):
        """Lock when flushing logging messages."""
        if self._lock:
            with self._lock:
                super(MultiprocessingHandler, self).flush()
        else:
            super(MultiprocessingHandler, self).flush()

    def createLock(self):
        """logging by default uses threading, use a multiprocessing lock instead."""
        self.lock = None
        self._lock = multiprocessing.Lock()

    def aquire(self):
        """Disable."""
        pass

    def release(self):
        """Disable."""
        pass

def init_logging(level=logging.INFO):
    """
    Call this function to initialize the MooseDocs logging formatter.
    """

    # Custom format that colors and counts errors/warnings
    formatter = MooseDocsFormatter()
    handler = MultiprocessingHandler()
    handler.setFormatter(formatter)

    # Setup the custom formatter
    log = logging.getLogger('MooseDocs')
    log.addHandler(handler)

    log.setLevel(level)
    MooseDocs.LOG_LEVEL = level
    return log

def report_exception(msg, *args):
    """Helper to output exceptions in logs."""
    msg = msg.format(*args)
    msg += u'\n{}\n'.format(mooseutils.colorText(traceback.format_exc(), 'GREY'))
    return msg
