#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
from .SQAReport import SQAReport
from .LogHelper import LogHelper
from .check_unit_test_sqa import check_unit_test_sqa


class SQAUnitTestReport(SQAReport):
    """
    Report of GoogleTest SQA traceability metadata for a single unit/src directory.
    """

    def __init__(self, **kwargs):
        self.unit_root = kwargs.pop("unit_root")
        self.legacy_manifest = kwargs.pop("legacy_manifest")
        # Explicit list restricting the check to this directory's own files. This is
        # required (rather than relying on git_ls_files' cwd-scoping) for the repository
        # root case, where scoping by cwd alone would also sweep in every other module's
        # nested unit/src directory.
        self.tracked_files = kwargs.pop("tracked_files", None)
        super().__init__(**kwargs)

    def execute(self, **kwargs):
        """Run check_unit_test_sqa and log the resulting diagnostics."""
        logger = LogHelper(__name__, "unit_test_sqa", **kwargs)
        diagnostics = check_unit_test_sqa(
            self.unit_root,
            legacy_manifest=self.legacy_manifest,
            tracked_files=self.tracked_files,
        )
        for diagnostic in diagnostics:
            logger.log(
                "unit_test_sqa",
                "{}:{}: {}",
                diagnostic.filename,
                diagnostic.line,
                diagnostic.message,
            )
        return logger
