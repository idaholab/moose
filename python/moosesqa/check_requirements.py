#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import re
import os
import collections
import logging
import enum
import pyhit
import mooseutils
import moosesqa
from .Requirement import Requirement
from .LogHelper import LogHelper

ISSUE_RE = re.compile(r'^([0-9a-f]{6,40})$|^(.*#[0-9]+)$', flags=re.MULTILINE)

class RequirementLogHelper(LogHelper):
    COLOR_TEXT = True

    def log(self, key, req, msg, *args, filename=None, name=None, line=None, **kwargs):
        msg = self._colorTestInfo(req, filename, name, line) + msg
        LogHelper.log(self, key, msg, *args, **kwargs)

    @staticmethod
    def _colorTestInfo(req, filename, name, line):
        """Helper for creating first line of message with file:test:line information"""
        filename = filename or req.filename
        name = mooseutils.colorText(name or req.name, 'MAGENTA', colored=RequirementLogHelper.COLOR_TEXT)
        line = mooseutils.colorText(str(line if (line is not None) else req.line), 'CYAN', colored=RequirementLogHelper.COLOR_TEXT)
        return '{}:{}:{}\n'.format(filename, name, line)

def check_requirements(requirements, file_list=None, color_text=True, allowed_collections=None, allowed_classifications=None, **kwargs):
    """
    Tool for checking Requirement for deficiencies
    """

    # Create key values for the logging messages
    log_default = kwargs.get('log_default', logging.ERROR)
    kwargs.setdefault('log_deprecated_requirement', log_default)
    kwargs.setdefault('log_deprecated_design', log_default)
    kwargs.setdefault('log_deprecated_issues', log_default)
    kwargs.setdefault('log_deprecated_detail', log_default)
    kwargs.setdefault('log_deprecated_verification', log_default)
    kwargs.setdefault('log_deprecated_validation', log_default)
    kwargs.setdefault('log_deprecated_with_details', log_default)
    kwargs.setdefault('log_missing', log_default)
    kwargs.setdefault('log_missing_requirement', log_default)
    kwargs.setdefault('log_missing_design', log_default)
    kwargs.setdefault('log_missing_issues', log_default)
    kwargs.setdefault('log_empty_requirement', log_default)
    kwargs.setdefault('log_empty_design', log_default)
    kwargs.setdefault('log_empty_issues', log_default)
    kwargs.setdefault('log_empty_verification', log_default)
    kwargs.setdefault('log_empty_validation', log_default)
    kwargs.setdefault('log_top_level_detail', log_default)
    kwargs.setdefault('log_missing_detail', log_default)
    kwargs.setdefault('log_empty_detail', log_default)
    kwargs.setdefault('log_extra_requirement', log_default)
    kwargs.setdefault('log_extra_design', log_default)
    kwargs.setdefault('log_extra_issues', log_default)
    kwargs.setdefault('log_extra_collections', log_default)
    kwargs.setdefault('log_invalid_collection', log_default)
    kwargs.setdefault('log_issue_format', log_default)
    kwargs.setdefault('log_design_files', log_default)
    kwargs.setdefault('log_validation_files', log_default)
    kwargs.setdefault('log_verification_files', log_default)
    kwargs.setdefault('log_testable', log_default)
    kwargs.setdefault('log_duplicate_requirement', log_default)
    kwargs.setdefault('log_duplicate_detail', log_default)

    logger = RequirementLogHelper(__name__, **kwargs)
    RequirementLogHelper.COLOR_TEXT = color_text

    # Setup file_list, if not provided
    if (file_list is None) and (not mooseutils.git_is_repo()):
        msg = "If the 'file_list' is not provided then the working directory must be a git repository."
        raise ValueError(msg)
    elif file_list is None:
        root = mooseutils.git_root_dir()
        ver = mooseutils.git_version()
        file_list = mooseutils.git_ls_files(root, recurse_submodules=True)

    # Setup allowed collections
    if allowed_collections is None:
        allowed_collections = set(moosesqa.MOOSESQA_COLLECTIONS)

    # Storage container for duplicate detection
    requirement_dict = collections.defaultdict(set)

    # Check each Requirement object for deficiencies
    for req in requirements:
        _check_requirement(req, logger, file_list, allowed_collections)
        if req.requirement is not None:
            key = [req.requirement]
            for detail in req.details:
                if detail.detail is not None:
                    key.append(detail.detail)
            requirement_dict['\n'.join(key)].add(req)

    # Duplicate checking
    for txt, value in requirement_dict.items():
        if len(value) > 1:
            msg = 'Duplicate requirements found:'
            msg += '\n{}\n'.format(mooseutils.colorText(txt, 'GREY', colored=color_text))
            for r in value:
                r.duplicate = True
                msg += RequirementLogHelper._colorTestInfo(r, None, None, None)

            LogHelper.log(logger, 'log_duplicate_requirement', msg.strip('\n'))

    return logger

def _check_requirement(req, logger, file_list, allowed_collections):
    """Opens tests specification and extracts requirement items."""

    # Test for 'deprecated' with other parameters
    if req.deprecated:
        if req.requirement is not None:
            logger.log('log_deprecated_requirement', req, "Deprecated test with 'requirement'", line=req.requirement_line)

        if req.design is not None:
            logger.log('log_deprecated_design', req, "Deprecated test with 'design'", line=req.design_line)

        if req.issues is not None:
            logger.log('log_deprecated_issues', req, "Deprecated test with 'issues'", line=req.issues_line)

        if req.verification is not None:
            logger.log('log_deprecated_verification', req, "Deprecated test with 'verification'", line=req.verification_line)

        if req.validation is not None:
            logger.log('log_deprecated_validation', req, "Deprecated test with 'validation'", line=req.validation_line)

        if req.detail is not None:
            logger.log('log_deprecated_detail', req, "Deprecated test with 'detail'", line=req.detail_line)

        if req.details:
            logger.log('log_deprecated_with_details', req, "Deprecated test with sub-block(s)")

    else:

        # Missing all three parameters
        if (req.requirement is None) and (req.design is None) and (req.issues is None):
            logger.log('log_missing', req, "No 'requirement', 'design', and 'issues' supplied")

        # Missing parameter(s)
        if (req.requirement is None) and ((req.design is not None) or (req.issues is not None)):
            logger.log('log_missing_requirement', req, "No 'requirement' supplied")

        if (req.design is None) and ((req.requirement is not None) or (req.issues is not None)):
            logger.log('log_missing_design', req, "No 'design' supplied")

        if (req.issues is None) and ((req.requirement is not None) or (req.design is not None)):
            logger.log('log_missing_issues', req, "No 'issues' supplied")

        # Test for empty
        if req.requirement == '':
            logger.log('log_empty_requirement', req, "Empty 'requirement' supplied", line=req.requirement_line)

        if req.design == []:
            logger.log('log_empty_design', req, "Empty 'design' supplied", line=req.design_line)

        if req.issues == []:
            logger.log('log_empty_issues', req, "Empty 'issues' supplied", line=req.issues_line)

        # Empty verification/validation
        if req.verification == []:
            logger.log('log_empty_verification', req, "Empty 'verification' supplied", line=req.verification_line)

        if req.validation == []:
            logger.log('log_empty_validation', req, "Empty 'validation' supplied", line=req.validation_line)

        # Test for 'detail' at top level
        if hasattr(req, 'detail') and req.detail is not None:
            logger.log('log_top_level_detail', req, "Top level 'detail' supplied", line=req.detail_line)

        # Test for 'detail'
        for detail in req.details:

            # Missing/empty 'detail'
            if detail.detail is None:
                logger.log('log_missing_detail', detail, "No 'detail' supplied")

            elif detail.detail == '':
                logger.log('log_empty_detail', detail, "Empty 'detail' supplied", line=detail.detail_line)

            # Extra 'requirement', 'design', 'issues', and/or 'deprecated'
            if hasattr(detail, 'requirement') and detail.requirement is not None:
                logger.log('log_extra_requirement', detail, "Extra 'requirement' supplied", line=detail.requirement_line)

            if hasattr(detail, 'design') and detail.design is not None:
                logger.log('log_extra_design', detail, "Extra 'design' supplied", line=detail.design_line)

            if hasattr(detail, 'issues') and detail.issues is not None:
                logger.log('log_extra_issues', detail, "Extra 'issues' supplied", line=detail.issues_line)

            if hasattr(detail, 'collections') and detail.collections is not None:
                logger.log('log_extra_collections', detail, "Extra 'collections' supplied", line=detail.collections_line)

            if hasattr(detail, 'deprecated') and detail.deprecated:
                logger.log('log_deprecated_detail', detail, "Sub-block with 'deprecated' supplied", line=detail.issues_line)

        # Test format of 'issues'
        if req.issues is not None:
            for issue in req.issues:
                if not ISSUE_RE.fullmatch(issue):
                    msg = "Invalid format for 'issues' supplied ({})\n".format(issue)
                    msg += 'Issue entries must be a issue number (e.g., #1235), an issues number with a repository name (e.g., libmesh#12345), or at least six digits of a git commit'
                    logger.log('log_issue_format', req, msg, line=req.issues_line)

        # Test that files in design/verification/validation exist
        if req.design is not None:
            for fname in req.design:
                if not _has_file(fname, file_list):
                    msg = "Unable to locate 'design' document ending with {}".format(fname)
                    logger.log('log_design_files', req, msg, line=req.design_line)

        if req.verification is not None:
            for fname in req.verification:
                if not _has_file(fname, file_list):
                    msg = "Unable to locate 'verification' document ending with {}".format(fname)
                    logger.log('log_verification_files', req, msg, line=req.verification_line)

        if req.validation is not None:
            for fname in req.validation:
                if not _has_file(fname, file_list):
                    msg = "Unable to locate 'validation' document ending with {}".format(fname)
                    logger.log('log_validation_files', req, msg, line=req.validation_line)

        # Test for testable
        if not req.testable:
            msg = "Test will not execute because it is marked as skipped or deleted"
            logger.log('log_testable', req, msg)

        # Test for duplicate details
        details_dict = collections.defaultdict(set)
        for detail in req.details:
            if detail.detail is not None:
                details_dict[detail.detail].add(detail)
        for txt, value in details_dict.items():
            if len(value) > 1:
                msg = 'Duplicate details found:'
                msg += '\n{}\n'.format(mooseutils.colorText(txt, 'GREY'))
                for r in value:
                    r.duplicate = True
                    msg += RequirementLogHelper._colorTestInfo(r, None, None, None)
                LogHelper.log(logger, 'log_duplicate_detail', msg.strip('\n'))

        # Test for invalid 'collections'
        if (allowed_collections is not None) and (req.collections):
            wrong = req.collections.difference(allowed_collections)
            if wrong:
                msg = 'Invalid collection names found: {}'.format(' '.join(wrong))
                LogHelper.log(logger, 'log_invalid_collection', msg)

def _has_file(filename, file_list):
    """Test if the filename is located in the list"""
    for f in file_list:
        if f.endswith(filename):
            return True
    return False
