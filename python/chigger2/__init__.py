#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import traceback
import logging
from moosetools import mooseutils

class ChiggerFormatter(logging.Formatter):
    """
    A formatter that is aware of the class hierarchy of the MooseDocs library.
    Call the init_logging function to initialize the use of this custom formatter.

    TODO: ChiggerFormatter or something similar (MooseDocsFormatter) should be used by all
          moosetools as should be the logging methods in ChiggerObject.

          Perhaps a "mixins" package: 'moosetools.mixins.MooseLoggerMixin' would add the log methods,
          other objects such at the AutoProperty would also go within that module
    """
    COLOR = dict(DEBUG='cyan_1',
                 INFO='white',
                 WARNING='yellow_1',
                 ERROR='red_1',
                 CRITICAL='magenta_1')

    COUNTS = dict(CRITICAL=0, ERROR=0, WARNING=0, INFO=0, DEBUG=0)

    def format(self, record):
        """Format the supplied logging record and count the occurrences."""
        self.COUNTS[record.levelname] += 1
        msg = '{}: {}'.format(mooseutils.color_text(record.levelname, self.COLOR[record.levelname]),
                              logging.Formatter.format(self, record))


        #if record.levelno > logging.INFO:
        #    stack = traceback.extract_stack()
        #    msg += '\n{}:{}\n'.format(mooseutils.color_text(stack[0].filename, 'dodger_blue_1'),
        #                              mooseutils.color_text(str(stack[0].lineno), 'grey_70'))
        #    msg += '{0}\n{1}\n{0}\n'.format('━' * len(stack[0].line), stack[0].line)
        #    msg += '\n{}\n'.format(mooseutils.color_text(''.join(stack.format()), 'grey_30'))
        return msg

# Setup the logging
level = dict(critical=logging.CRITICAL, error=logging.ERROR, warning=logging.warning,
             info=logging.INFO, debug=logging.DEBUG, notset=logging.NOTSET)

formatter = ChiggerFormatter()
handler = logging.StreamHandler()
handler.setFormatter(formatter)

log = logging.getLogger('')
log.addHandler(handler)
log.setLevel(level[os.getenv('CHIGGER_LOG_LEVEL', 'INFO').lower()])

from .Window import Window
from .Viewport import Viewport, Background
from . import annotations
from . import base
from . import utils
from . import misc
from . import exodus
from . import geometric
from . import graphs
from . import filters
from . import observers
