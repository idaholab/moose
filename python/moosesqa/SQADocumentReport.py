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
import urllib
import logging
import glob
import mooseutils
from .SQAReport import SQAReport
from .get_documents import get_documents, INL_DOCUMENTS
from .check_documents import check_documents

@mooseutils.addProperty('working_dirs', ptype=list)
@mooseutils.addProperty('required_documents', ptype=list)
class SQADocumentReport(SQAReport):
    """
    Report for existence of required SQA documents and links.
    """
    def __init__(self, **kwargs):
        self._documents = dict()
        kwargs.setdefault('required_documents', INL_DOCUMENTS)
        for name in kwargs.get('required_documents'):
            doc = kwargs.pop(name, None)
            if doc is not None:
                doc = mooseutils.eval_path(doc)
            self._documents[name] = doc

        super().__init__(**kwargs)
        if self.working_dirs is None: self.working_dirs = [mooseutils.git_root_dir()]

    @property
    def documents(self):
        return get_documents(self.required_documents, **self._documents)

    def execute(self, **kwargs):
        """Determine the status"""
        file_list = list()
        for working_dir in self.working_dirs:
            path = mooseutils.eval_path(working_dir)
            if mooseutils.is_git_repo(path):
                file_list += mooseutils.git_ls_files(path)
            else:
                file_list += glob.glob(os.path.join(path,'**', '*.*'), recursive=True)

        logger = check_documents(self.documents, file_list, **kwargs)
        return logger
