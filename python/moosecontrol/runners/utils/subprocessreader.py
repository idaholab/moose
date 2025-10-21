# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines the SubprocessReader."""

from logging import getLogger
from threading import Thread
from subprocess import Popen

logger = getLogger('SubprocessReader')

class SubprocessReader(Thread):
    """
    Thread that reads stdout from a subprocess
    and pipes it to a log.
    """
    # Prefix appended to output
    OUTPUT_PREFIX: str = 'OUTPUT: '

    def __init__(self, process: Popen):
        """
        Parameters
        ----------
        process : Popen
            The process to read stdout from.
        """
        assert isinstance(process, Popen)

        super().__init__()

        # The process
        self._process: Popen = process

    def run(self):
        """
        Run the thread.
        """
        logger.info('Subprocess reader started')

        assert self._process.universal_newlines
        assert self._process.poll() is None

        pipe = self._process.stdout
        assert pipe is not None

        with pipe:
            for line in iter(pipe.readline, ''):
                logger.info(self.OUTPUT_PREFIX + line.rstrip())

        logger.info('Subprocess reader ending')
