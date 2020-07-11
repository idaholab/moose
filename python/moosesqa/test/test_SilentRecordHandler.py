#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import io
import sys
import subprocess
import unittest
import logging
import mooseutils
import moosesqa

class TestSilentRecordHandler(unittest.TestCase):
    def testHandler(self):
        logger = logging.getLogger(__name__)
        handler = moosesqa.SilentRecordHandler()
        logger.addHandler(handler)

        logger.error("error")
        logger.warning('warning')

        records = handler.getRecords()
        self.assertEqual(len(records[logging.ERROR]), 1)
        self.assertEqual(list(records[logging.ERROR])[0].getMessage(), 'error')
        self.assertEqual(len(records[logging.WARNING]), 1)
        self.assertEqual(list(records[logging.WARNING])[0].getMessage(), 'warning')

        handler.clearRecords()
        records = handler.getRecords()
        self.assertEqual(len(records[logging.ERROR]), 0)
        self.assertEqual(len(records[logging.WARNING]), 0)


if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
