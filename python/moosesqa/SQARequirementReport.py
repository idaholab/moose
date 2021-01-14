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
import copy
import pyhit
import collections
import logging

import mooseutils
from .get_requirements import get_requirements_from_tests
from .check_requirements import check_requirements, RequirementLogHelper
from .SQAReport import SQAReport
from .LogHelper import LogHelper

@mooseutils.addProperty('working_dir', ptype=str)
@mooseutils.addProperty('directories', ptype=list)
@mooseutils.addProperty('specs', ptype=str)
@mooseutils.addProperty('test_names', ptype=set)
@mooseutils.addProperty('global_report', ptype=bool, default=False)
class SQARequirementReport(SQAReport):
    """
    Data wrapper for SQA requirement/design/issue information.
    """
    def execute(self, **kwargs):
        """
        Computes the percent complete, number missing items, and the pass/fail status
        """

        # Extract configuration parameters
        working_dir = self.working_dir or mooseutils.git_root_dir()
        local_dirs = self.directories
        specs = self.specs or 'tests'

        # Get complete directory paths
        if local_dirs:
            directories = list()
            for local_dir in local_dirs:
                d = mooseutils.eval_path(local_dir)
                if not os.path.isdir(d):
                    d = os.path.join(working_dir, d)
                directories.append(d)
        else:
            directories = [working_dir]

        # Check that directories exist
        for d in directories:
            if not os.path.isdir(d):
                raise NotADirectoryError("Supplied directory does not exist: {}".format(d))

        # Build Requirement objects and remove directory based dict
        req_dict = get_requirements_from_tests(directories, specs.split())
        requirements = []
        for values in req_dict.values():
            requirements += values

        # Populate the lists of tests for SQARequirementDiffReport
        self.test_names = set()
        for req in requirements:
            self.test_names.add((req.filename, req.name, req.line))

        # Check the requirements
        logger = check_requirements(requirements, color_text=self.color_text, **kwargs)
        return logger

@mooseutils.addProperty('reports', ptype=list)
class SQARequirementDiffReport(SQAReport):
    """
    Report for SQA missing requirement/design/issue information.
    """
    def execute(self, **kwargs):

        # Create/run the repository level report
        global_report = SQARequirementReport(log_default="none")
        global_report.getReport()

        # Create the LogHelper for the diff report
        kwargs.setdefault('log_missing_test', logging.ERROR)
        logger = LogHelper(__name__, **kwargs)

        # Tests form local reports
        local_names = set()
        for report in self.reports:
            if not report.test_names:
                report.execute()
            local_names.update(report.test_names)

        # Compute the difference and report missing tests
        missing_tests = global_report.test_names.difference(local_names)
        for filename, name, line in missing_tests:
            msg = RequirementLogHelper._colorTestInfo(None, filename, name, line)
            logger.log('log_missing_test', msg.strip('\n'))

        self.number_of_newlines_after_log = 1 # print all the missing tests without extra space
        return logger
