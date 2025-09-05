#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
import mooseutils
from .SQAReport import SQAReport
from .get_documents import get_documents, INL_DOCUMENTS, DEPRECATED_DOCUMENTS
from .check_documents import check_documents


class SQADocumentReport(SQAReport):
    """
    Report for existence of required SQA documents and links.
    """

    def __init__(self, **kwargs):
        kwargs.setdefault("required_documents", INL_DOCUMENTS)
        kwargs.setdefault("deprecated_documents", DEPRECATED_DOCUMENTS)
        self.working_dirs = kwargs.pop("working_dirs", None)
        self.required_documents = kwargs.pop("required_documents", None)
        self.deprecated_documents = kwargs.pop("deprecated_documents", None)
        self.valid_deprecations = dict()

        self._documents = dict()
        for name in self.required_documents:
            doc = kwargs.pop(name, None)
            if doc is not None:
                doc = mooseutils.eval_path(doc)
            self._documents[name] = doc

        # Check for deprecated document names and add to documents dict under new name
        # Mark deprecated document name for entry into the log (see check_documents)
        for dep_name, new_name in self.deprecated_documents.items():
            doc = kwargs.pop(dep_name, None)
            if doc is not None:
                doc = mooseutils.eval_path(doc)
                self.valid_deprecations[dep_name] = new_name
                self._documents[new_name] = doc

        super().__init__(**kwargs)
        if self.working_dirs is None:
            self.working_dirs = [mooseutils.git_root_dir()]

    @property
    def documents(self):
        return get_documents(self.required_documents, **self._documents)

    def execute(self, **kwargs):
        """Determine the status"""
        file_list = SQAReport._getFiles(self.working_dirs)
        logger = check_documents(
            self.documents, self.valid_deprecations, file_list, **kwargs
        )
        return logger
