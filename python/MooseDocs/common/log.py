# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Helper for defining custom MooseDocs logging."""
import logging
import traceback
import multiprocessing
from logging import LogRecord  # for Type Hinting
import mooseutils
import moosesqa
import MooseDocs


class MooseDocsFormatter(logging.Formatter):
    """
    A formatter that is aware of the class hierarchy of the MooseDocs library.
    Call the init_logging function to initialize the use of this custom formatter.
    """

    COLOR = {
        "DEBUG": "CYAN",
        "INFO": "RESET",
        "WARNING": "LIGHT_YELLOW",
        "ERROR": "LIGHT_RED",
        "CRITICAL": "MAGENTA",
    }

    def format(self, record: LogRecord) -> str:
        """Format the supplied logging record and count the occurrences."""
        tid = multiprocessing.current_process().name
        msg = f"{record.name} ({tid}): {logging.Formatter.format(self, record)}"
        return mooseutils.colorText(msg, self.COLOR[record.levelname])


class MultiprocessingHandler(logging.StreamHandler):
    """A simple handler that locks when writing with multiprocessing."""

    COUNTS = {
        logging.CRITICAL: multiprocessing.Value("I", 0, lock=True),
        logging.ERROR: multiprocessing.Value("I", 0, lock=True),
        logging.WARNING: multiprocessing.Value("I", 0, lock=True),
        logging.INFO: multiprocessing.Value("I", 0, lock=True),
        logging.DEBUG: multiprocessing.Value("I", 0, lock=True),
    }

    # Initialize StreamHandler locks (Python 3.13 requirement)
    stream_handler = logging.StreamHandler()

    def get_count(self, level: int) -> int:
        """return error counts for supplied level"""
        return MultiprocessingHandler.COUNTS[level].value

    def handle(self, record: LogRecord) -> None:
        super().handle(record)
        with MultiprocessingHandler.COUNTS[record.levelno].get_lock():
            MultiprocessingHandler.COUNTS[record.levelno].value += 1

    def flush(self) -> None:
        """Lock when flushing logging messages."""
        if self._lock:
            with self._lock:
                super().flush()
        else:
            super().flush()

    def createLock(self) -> None:
        """logging by default uses threading, use a multiprocessing lock instead."""
        self.lock = self.stream_handler.lock
        self._lock = multiprocessing.Lock()

    def aquire(self) -> None:
        """Disable."""

    def release(self) -> None:
        """Disable."""


def init_logging(level: int = logging.INFO, silent: bool = False) -> None:
    """
    Call this function to initialize the MooseDocs logging formatter.
    """

    # Custom format that colors and counts errors/warnings
    if silent:
        handler = moosesqa.SilentRecordHandler()
    else:
        handler = MultiprocessingHandler()

    formatter = MooseDocsFormatter()
    handler.setFormatter(formatter)

    # Setup the custom formatter
    log = logging.getLogger("MooseDocs")
    log.addHandler(handler)
    log.setLevel(level)

    MooseDocs.LOG_LEVEL = level


def report_exception(msg: str, *args) -> str:
    """Helper to output exceptions in logs."""
    msg = msg.format(*args)
    msg += f'\n{mooseutils.colorText(traceback.format_exc(), "GREY")}\n'
    return msg
