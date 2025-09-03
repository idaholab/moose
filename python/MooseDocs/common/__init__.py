# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""
Module for objects and functions that are commonly used throughout the MooseDocs system.
"""
from .storage import Storage as Storage

from .parse_settings import (
    match_settings as match_settings,
    parse_settings as parse_settings,
    get_settings_as_dict as get_settings_as_dict,
)

from .box import box as box

from .load_config import (
    load_config as load_config,
    load_configs as load_configs,
    load_extensions as load_extensions,
)

from .build_class_database import build_class_database as build_class_database

from .read import (
    read as read,
    write as write,
    get_language as get_language,
)

from .regex import regex as regex

from .project_find import project_find as project_find

from .check_filenames import check_filenames as check_filenames

from .extract_content import (
    extractContent as extractContent,
    extractContentSettings as extractContentSettings,
    fix_moose_header as fix_moose_header,
)

from .log import report_exception as report_exception

from .report_error import report_error as report_error

from .exceptions import MooseDocsException as MooseDocsException

from .get_content import (
    get_content as get_content,
    get_files as get_files,
    create_file_page as create_file_page,
    get_items as get_items,
)

from .has_tokens import has_tokens as has_tokens

from .template import apply_template_arguments as apply_template_arguments
