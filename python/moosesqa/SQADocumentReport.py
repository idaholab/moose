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
import mooseutils
from .SQAReport import SQAReport
from .get_documents import get_documents, INL_DOCUMENTS
from .check_documents import check_documents

@mooseutils.addProperty('working_dir', ptype=str)
@mooseutils.addProperty('required_documents', ptype=list)
class SQADocumentReport(SQAReport):
    """
    Report for existence of required SQA documents and links.
    """
    def __init__(self, **kwargs):
        self._documents = dict()
        kwargs.setdefault('required_documents', INL_DOCUMENTS)
        self._documents = {name:kwargs.pop(name, None) for name in kwargs.get('required_documents')}
        super().__init__(**kwargs)
        self.working_dir = self.working_dir or mooseutils.git_root_dir()

    def execute(self, **kwargs):
        """Determine the status"""
        documents = get_documents(self.required_documents, **self._documents)
        logger = check_documents(documents, None, **kwargs)
        return logger
