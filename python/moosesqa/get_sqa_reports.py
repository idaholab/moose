#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
import copy
import subprocess
import logging
import mooseutils
import moosetree
import pyhit

LOG = logging.getLogger(__name__)

from .SQARequirementReport import SQARequirementReport, SQARequirementDiffReport
from .SQADocumentReport import SQADocumentReport
from .SQAMooseAppReport import SQAMooseAppReport
from .SQAUnitTestReport import SQAUnitTestReport
from .check_unit_test_sqa import discover_unit_test_directories, is_moose_repository


def get_sqa_reports(config_file, app_report=True, doc_report=True, req_report=True):
    """
    Generate reports regarding SQA content.

    Input:
        config_file[str|dict]: A YAML file to load or loaded YAML dict object
        app/doc/req_report [bool]: Flags for controlling the creating of the various reports

    Output:
        doc_reports: List of SQADocumentReport objects for the existence of SQA documents links/files
        req_reports: List of SQARequirementReport objects test requirement information
        app_reports: List of SQAMooseAppReport objects for class documentation

    See moose/scripts/check_sqa.py for usage.
    """
    config = (
        mooseutils.yaml_load(config_file)
        if isinstance(config_file, str)
        else config_file
    )
    doc_reports = _get_sqa_document_reports(config) if doc_report else None
    req_reports = _get_sqa_requirement_reports(config) if req_report else None
    app_reports = _get_sqa_app_reports(config) if app_report else None
    return doc_reports, req_reports, app_reports


def _get_sqa_document_reports(config):
    """Helper function for building SQADocumentReport objects"""

    if "Documents" not in config:
        return None

    kwargs = config["Documents"]
    kwargs.setdefault("title", "Documents")
    doc_report = SQADocumentReport(**kwargs)
    return [doc_report]


def _get_sqa_requirement_reports(config):
    """Helper for building the SQARequirementReport objects"""

    if "Requirements" not in config:
        return None

    req_config = config["Requirements"]
    reports = list()

    # Local reports
    diff_report = req_config.pop("create_diff_report", False)
    for name, kwargs in req_config.items():
        kwargs.setdefault("title", name)
        local = SQARequirementReport(**kwargs)
        reports.append(local)

    # Local/global difference report
    if diff_report:
        diff = SQARequirementDiffReport(
            title="Missing Tests from Reports", reports=copy.copy(reports)
        )
        reports.append(diff)

    return reports


def _get_sqa_app_reports(config):
    """Helper for building the SQAMooseAppReport objects"""

    if "Applications" not in config:
        return None

    app_configs = config["Applications"]
    reports = list()
    for name, kwargs in app_configs.items():
        kwargs.setdefault("title", name)
        reports.append(SQAMooseAppReport(**kwargs))

    return reports


def get_sqa_unit_test_reports(root_dir=None):
    """
    Build one SQAUnitTestReport per directory containing a unit/src subdirectory.

    Input:
        root_dir[str]: repository root to scan (default: mooseutils.git_root_dir())

    Output:
        List of SQAUnitTestReport objects, one per discovered unit/src directory.
    """
    root_dir = root_dir or mooseutils.git_root_dir()
    # Gathered once and filtered per module below. This can't be left to each report's
    # own tracked_files=None default: 'unit/src/...' is matched at any depth, so scoping
    # by cwd alone would make the repository-root ('' module) check also sweep in every
    # other module's nested unit/src directory.
    tracked_files = list(mooseutils.git_ls_files(root_dir))
    # Computed once and reused for every module filtered below, rather than recomputing
    # relpath() for every tracked file on every module iteration (O(modules * files)).
    relative_tracked = [
        (filename, os.path.relpath(filename, root_dir).replace(os.sep, "/"))
        for filename in tracked_files
    ]
    # For the root-level ("") module, 'framework' is only a meaningful title in the MOOSE
    # repository itself; a downstream app's root-level unit tests are named for its own
    # repository instead.
    root_title = (
        "framework" if is_moose_repository(root_dir) else os.path.basename(root_dir)
    )
    reports = list()
    for module_root, legacy_manifest in discover_unit_test_directories(
        root_dir, tracked_files=tracked_files
    ):
        prefix = module_root + "/" if module_root else "unit/"
        module_tracked = [
            filename
            for filename, relative in relative_tracked
            if relative.startswith(prefix)
        ]
        reports.append(
            SQAUnitTestReport(
                title=module_root or root_title,
                unit_root=(
                    os.path.join(root_dir, module_root) if module_root else root_dir
                ),
                legacy_manifest=legacy_manifest,
                tracked_files=module_tracked,
            )
        )
    return reports
