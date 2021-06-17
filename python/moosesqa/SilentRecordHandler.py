#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import collections
import copy
import logging
class SilentRecordHandler(logging.NullHandler):
    """Custom logging Handler object for caching warnings/errors"""
    COUNTS = collections.defaultdict(int)

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._records = collections.defaultdict(set)

    def getCount(self, level):
        return SilentRecordHandler.COUNTS[level]

    def getRecords(self):
        return copy.copy(self._records)

    def clear(self):
        self.clearRecords()
        self.clearCounts()

    def clearRecords(self):
        self._records.clear()

    def clearCounts(self):
        self.COUNTS.clear()

    def handle(self, record):
        self._records[record.levelno].add(record)
        self.COUNTS[record.levelno] += 1
