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
import moosetree
import moosesyntax
from .LogHelper import LogHelper

def check_syntax(app_syntax, app_types, file_cache, object_prefix='', syntax_prefix='', **kwargs):

    kwargs.setdefault('log_hidden_syntax', logging.ERROR)
    kwargs.setdefault('log_stub_files', logging.ERROR)
    kwargs.setdefault('log_duplicate_files', logging.ERROR)
    kwargs.setdefault('log_removed_and_hidden', logging.ERROR)
    kwargs.setdefault('log_missing_description', logging.ERROR)
    kwargs.setdefault('log_missing_markdown', logging.ERROR)
    kwargs.setdefault('log_hidden_non_stub', logging.ERROR)
    kwargs.setdefault('log_non_hidden_stub', logging.ERROR)
    kwargs.setdefault('log_duplicate_files', logging.ERROR)
    logger = LogHelper(__name__, **kwargs)

    func = lambda n: any([app_type in n.groups() for app_type in app_types]) and len(n.groups()) == 1
    for node in moosetree.iterate(app_syntax, func=func):
        _check_node(node, file_cache, object_prefix, syntax_prefix, logger)

    return logger

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
    is_stub = file_is_stub(md_file)
    is_missing_description = is_object and (node.description is None) and (not node.removed)
    is_missing = (md_file is None) and (not node.removed)

    # ERROR: object is hidden
    if node.hidden:
        msg = "{} is marked as hidden, documentation is considered incomplete if objects are hidden.".format(node.fullpath())
        logger.log('log_hidden_syntax', msg)

    # ERROR: object is stub
    if is_stub:
        msg = "{} is a stub file, documentation is considered incomplete if pages are unmodified from stubs.".format(node.fullpath())
        logger.log('log_stub_files', msg)

    # ERROR: object is hidden and removed
    if node.removed and node.hidden:
        msg = "{} is marked as both hidden and removed.".format(node.fullpath())
        logger.log('log_removed_and_hidden', msg)

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

    # ERROR: object is hidden, but markdown is not a stub
    if (not node.removed) and (node.hidden and (md_file is not None) and (not is_stub)):
        msg = "{} is hidden but the content is not a stub.".format(node.fullpath())
        logger.log('log_hidden_non_stub', msg)

    # ERROR: markdown is a stub but not hidden
    if (not node.removed) and (md_file is not None) and (is_stub and (not node.hidden)):
        msg = "{} has a stub markdown page and is not hidden.".format(node.fullpath())
        logger.log('log_non_hidden_stub', msg)

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
    if filename is not None:
        with open(filename, 'r') as fid:
            content = fid.read()
            if content and re.search(r'(stubs\/moose_(object|action|system).md.template)', content) or \
               ('!! MOOSE Documentation Stub (remove this when content is added)' in content):
                return True
    return False
