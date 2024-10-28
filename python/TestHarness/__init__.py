#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import sys
if sys.version_info < (3, 6):
    print('"TestHarness" requires python version 3.6 or greater, version {}.{} is being used.' \
          .format(sys.version_info[0], sys.version_info[1]))
    sys.exit(1)

from .TestHarness import TestHarness
from .OutputInterface import OutputInterface
from .TestHarness import findDepApps
__all__=['TestHarness', 'findDepApps']
