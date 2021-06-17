#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import urllib
import logging
import collections
import mooseutils
from .LogHelper import LogHelper

def check_documents(documents, file_list=None, **kwargs):
    """
    Tool for checking SQA document deficiencies
    """

    # Setup logger, assume the names of the documents with a "log_" prefix are the logging flags (see get_documents)
    log_default = kwargs.get('log_default', logging.ERROR)
    for doc in documents:
        kwargs.setdefault("log_" + doc.name, log_default)
    logger = LogHelper(__name__, **kwargs)

    # Setup file_list, if not provided
    if (file_list is None) and (not mooseutils.git_is_repo()):
        msg = "If the 'file_list' is not provided then the working directory must be a git repository."
        raise ValueError(msg)
    elif file_list is None:
        root = mooseutils.git_root_dir()
        file_list = mooseutils.git_ls_files(root, recurse_submodules=False)

    # Perform document checks
    for doc in documents:
        _check_document(doc.name, doc.filename, file_list, logger)

    return logger

def _check_document(name, filename, file_list, logger):
    """Helper for inspecting document"""
    log_key = "log_" + name

    if filename is None:
        msg = "Missing value for '{}' document: {}".format(name, filename)
        logger.log(log_key, msg)

    elif filename.startswith('http'):
        try:
            response = urllib.request.urlopen(filename)
        except urllib.error.URLError:
            msg = "Invalid URL for '{}' document: {}".format(name, filename)
            logger.log(log_key, msg)

    else:
        found = list()
        for fname in file_list:
            if fname.endswith(filename.split('#')[0]):
                found.append(filename)

        if len(found) == 0:
            msg = "Failed to locate '{}' document: {}".format(name, filename)
            logger.log(log_key, msg)
        elif len(found) > 1:
            msg = "Found multiple files for '{}' document:\n  ".format(name)
            msg += "\n  ".join(found)
            logger.log(log_key, msg)
