#* This file is part of MOOSETOOLS repository
#* https://www.github.com/idaholab/moosetools
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moosetools/blob/main/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import shutil
from moosetools import moosetest
from moosetools import mooseutils
from ..runners import MOOSEAppRunner

moosetest.controllers.TagController.AUTO_BUILD = False

class LegacyTagController(moosetest.controllers.TagController):
    """
    A `Controller` for command line test grouping (e.g., '--heavy').
    """
    AUTO_BUILD = True

    @staticmethod
    def validCommandLineArguments(parser, params):
        moosetest.controllers.TagController.validCommandLineArguments(parser, params)

        # Legacy Support
        parser.add_argument('--heavy', action='store_true', help="Only execute tests with 'HEAVY' tag.")

    def _setup(self, args):
        if args.heavy:
            args.tags = ['HEAVY']
        moosetest.controllers.TagController._setup(self, args)
