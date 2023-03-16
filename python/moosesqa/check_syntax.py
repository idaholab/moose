#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import re
import logging
import mooseutils
import moosetree
import moosesyntax
from .LogHelper import LogHelper

def check_syntax(app_syntax, app_types, file_cache, object_prefix='', syntax_prefix='', allow_test_objects=False, **kwargs):

    log_default = kwargs.get('log_default', logging.ERROR)
    kwargs.setdefault('log_stub_files', log_default)
    kwargs.setdefault('log_duplicate_files', log_default)
    kwargs.setdefault('log_missing_description', log_default)
    kwargs.setdefault('log_missing_markdown', log_default)
    kwargs.setdefault('log_duplicate_files', log_default)
    logger = LogHelper(__name__, **kwargs)

    for node in app_syntax.descendants:
        # SyntaxNode objects must registered to all the desired app types
        # ActionNode objects and the syntax they belong must be registered to the desired type
        # MooseObjects must be registered to the desired type
        # Action/MooseObjects must not be tests, unless allowed
        if ((isinstance(node, moosesyntax.SyntaxNode) and set(app_types) == node.groups()) \
            or (isinstance(node, moosesyntax.ActionNode) and is_app_type(node, app_types)) and is_app_type(node.parent, app_types) \
            or (isinstance(node, moosesyntax.MooseObjectNode) and is_app_type(node, app_types))) \
            and (allow_test_objects or not node.test):
            _check_node(node, file_cache, object_prefix, syntax_prefix, logger)

    return logger

def is_app_type(node, app_types):
    return any([app_type in node.groups() for app_type in app_types])

def _check_node(node, file_cache, object_prefix, syntax_prefix, logger):
    """Perform checks of a single Page node object"""

    # Do nothing for the root node or if the node is removed
    if node.is_root:
        return

    # True if it is a MooseObject node
    is_object = isinstance(node, moosesyntax.ObjectNodeBase)

    # Locate markdown file and determine if it is a stub page
    prefix = object_prefix if is_object else syntax_prefix
    md_path = os.path.join(prefix, node.markdown)
    md_file = find_md_file(md_path, file_cache, logger)
    is_missing_description = is_object and (not node.description) and (not node.removed)
    is_missing = ((md_file is None) or (not os.path.isfile(md_file))) and (not node.removed)
    is_stub = file_is_stub(md_file) if (not is_missing) and (not node.removed) else False

    # ERROR: object is stub
    if is_stub:
        msg = "{} is a stub file, documentation is considered incomplete if pages are unmodified from stubs.".format(node.fullpath())
        logger.log('log_stub_files', msg)

    # ERROR: object does not have a markdown file and is not removed
    if (not node.removed) and is_missing:
        msg = "{} is missing a markdown file.\n".format(node.fullpath())
        msg += "  The page should be located at {}\n".format(md_path)
        msg += "  A stub page can be created using using the './moosedocs.py generate' command"
        logger.log('log_missing_markdown', msg)

    # ERROR: missing description
    if (not node.removed) and is_missing_description:
        msg = "{} is missing a class description.".format(node.fullpath())
        logger.log('log_missing_description', msg)

    # Store attributes for stub page generation
    node['_md_file'] = md_file
    node['_md_path'] = md_path
    node['_is_stub'] = is_stub

def find_md_file(path, file_cache, logger):
    """Helper for locating a markdown file"""
    out = list()
    for filename in file_cache:
        if filename.endswith(path):
            out.append(filename)

    if len(out) > 1:
        # This should not be possible unless multiple content directories are included
        # in the file cache. In this case, it is possible to have more than one markdown file
        # match. For example, Distributions/index.md has a basic version in the framework that
        # is overridden by the detailed version in the stochastic tools module.
        msg = "Located multiple files for the desired markdown: {}:\n  ".format(path)
        msg += '\n  '.join(out)
        logger.log('log_duplicate_files', msg)

    if out:
        return out[0]

    return None

def file_is_stub(filename):
    """Helper for getting stub status for markdown file"""
    with open(filename, 'r') as fid:
        content = fid.read()

    # Empty is considered a stub
    if not content:
        return True
    # Old template method
    elif re.search(r'stubs/.*\.md\.template', content):
        return True
    # Even older comment method
    elif '!! MOOSE Documentation Stub (remove this when content is added)' in content:
        return True
    # Current alert method
    elif '!alert! construction title=Undocumented' in content:
        return True
    # Even more current alert method
    elif '!alert construction title=Undocumented' in content:
        return True
    return False
