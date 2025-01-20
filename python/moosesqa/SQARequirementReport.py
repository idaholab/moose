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

class SQARequirementReport(SQAReport):
    """
    Data wrapper for SQA requirement/design/issue information.
    """
    def __init__(self, **kwargs):
        self.working_dirs = kwargs.pop('working_dirs', None)
        self.directories = kwargs.pop('directories', None)
        self.specs = kwargs.pop('specs', None)
        self.test_names = kwargs.pop('test_names', None)
        self.global_report = kwargs.pop('global_report', False)
        self.include_non_testable = kwargs.pop('include_non_testable', False)
        SQAReport.__init__(self, **kwargs)

    def execute(self, **kwargs):
        """
        Computes the percent complete, number missing items, and the pass/fail status
        """

        # Extract configuration parameters
        specs = self.specs or 'tests'

        # Get complete directory paths
        root_dir = mooseutils.git_root_dir()
        directories = [mooseutils.eval_path(d) for d in (self.directories or [root_dir])]
        for i, d in enumerate(directories):
            if not os.path.isdir(d):
                directories[i] = os.path.join(root_dir, d)
            if not os.path.isdir(directories[i]):
                raise NotADirectoryError("Supplied directory does not exist: {}".format(d))

        # Build Requirement objects and remove directory based dict
        requirements = []
        for s in specs:
            req_dict = get_requirements_from_tests(directories, s.split(), self.include_non_testable)
            for values in req_dict.values():
                requirements += values

        # Populate the lists of tests for SQARequirementDiffReport
        self.test_names = set()
        for req in requirements:
            self.test_names.add((req.filename, req.name, req.line))

        # Get list of files to search
        if self.working_dirs is None: self.working_dirs = [mooseutils.git_root_dir()]
        file_list = SQAReport._getFiles(self.working_dirs)

        # Check the requirements
        logger = check_requirements(requirements, color_text=self.color_text, file_list=file_list, **kwargs)
        return logger

class SQARequirementDiffReport(SQAReport):
    """
    Report for SQA missing requirement/design/issue information.
    """
    def __init__(self, **kwargs):
        self.reports = kwargs.pop('reports', None)
        SQAReport.__init__(self, **kwargs)

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
