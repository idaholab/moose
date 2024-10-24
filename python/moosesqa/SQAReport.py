#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import enum
import logging
import collections
import copy
import mooseutils
from .LogHelper import LogHelper

LOG = logging.getLogger(__name__)

class SQAReport(object):
    """
    Base class for building reports regarding the status of SQA items.
    """

    class Status(enum.IntEnum):
        """Enum for indicating the report status."""
        PASS = 0
        WARNING = 1
        ERROR = 2

    def __init__(self, **kwargs):
        self.title = kwargs.pop('title', '')
        self.show_warning = kwargs.pop('show_warning', False)
        self.show_error = kwargs.pop('show_error', True)
        self.show_critical = kwargs.pop('show_critical', True)
        self.status = kwargs.pop('status', SQAReport.Status.PASS)
        self.logger = kwargs.pop('logger', None)
        self.color_text = kwargs.pop('color_text', True)
        self.number_of_newlines_after_log = kwargs.pop('number_of_newlines_after_log', 2)

        self.attributes = kwargs

        # SilentRecordHandler (added by moosesqa.__init__)
        self._handler = logging.getLogger('moosesqa').handlers[0]

        # All attributes are assumed to be LogHelper flags that are
        # automatically passed to the execute() method in getReport.
        for key, value in self.attributes.items():
            if not isinstance(value, str):
                raise RuntimeError("Unexpected value of '{}' for input '{}' , value must be CRITICAL, ERROR, WARNING, or NONE".format(value, key))
            if value.lower() == 'none':
                self.attributes[key] = None
            else:
                attr = getattr(logging, value.upper(), None)
                if attr is None:
                    raise RuntimeError("Unknown keyword argument '{}'".format(key))
                else:
                    self.attributes[key] = attr

    def getReport(self):
        """Generate the report."""

        # Execute the function to report about, getting the LogHelper in return
        self.logger = self.execute(**self.attributes)

        # Return the logging records from the SilentReportHandler
        records = self._handler.getRecords()
        self._handler.clearRecords()

        # Update status from logging
        if len(records[logging.WARNING]) > 0:
            self.status = 1

        if len(records[logging.ERROR]) > 0 or len(records[logging.CRITICAL]) > 0:
            self.status = 2

        # Determine the text length for the LogHelper keys
        width = 0
        for k in self.logger.modes.keys():
            width = max(width, len(k))

        # Start the report text
        text = '{} {}\n'.format(mooseutils.colorText(self.title, 'CYAN', colored=self.color_text),
                                self._getStatusText(self.status))

        # Report the error counts
        for key, mode in self.logger.modes.items():
            cnt = self.logger.counts[key]
            msg = '{:>{width}}: {}\n'.format(key, cnt, width=width+2)
            text += self._colorTextByMode(msg, mode) if cnt > 0 else msg
        text += '\n'

        # Add logged items to the report, if desired
        n = self.number_of_newlines_after_log
        if len(records[logging.CRITICAL]) > 0 and self.show_critical:
            text += self._colorTextByStatus('CRITICAL:\n', SQAReport.Status.ERROR)
            for record in records[logging.CRITICAL]:
                text += record.getMessage() + '\n'*n

        if len(records[logging.ERROR]) > 0 and self.show_error:
            text += self._colorTextByStatus('ERRORS:\n', SQAReport.Status.ERROR)
            for record in records[logging.ERROR]:
                text += record.getMessage() + '\n'*n

        if (len(records[logging.WARNING]) > 0) and self.show_warning:
            text += self._colorTextByStatus('WARNINGS:\n', SQAReport.Status.WARNING)
            for record in records[logging.WARNING]:
                text += record.getMessage() + '\n'*n

        return text.strip('\n')

    def execute(self, **kwargs):
        """
        (virtual) Method that performs the necessary operations for the report and returns LogHelper
        """
        raise NotImplementedError()

    def _colorTextByStatus(self, text, status):
        """Helper for coloring text based on status."""
        if not isinstance(text, str):
            text = str(text)
        color = 'LIGHT_GREEN'
        if status == SQAReport.Status.ERROR:
            color = 'LIGHT_RED'
        elif status == SQAReport.Status.WARNING:
            color = 'LIGHT_YELLOW'
        return mooseutils.colorText(text, color, colored=self.color_text)

    def _colorTextByMode(self, text, mode):
        if mode == logging.ERROR or mode == logging.CRITICAL:
            return mooseutils.colorText(text, 'LIGHT_RED', colored=self.color_text)
        elif mode == logging.WARNING:
            return mooseutils.colorText(text, 'LIGHT_YELLOW', colored=self.color_text)
        return text

    def _getStatusText(self, status):
        """Helper for returning a string version of the report status"""
        if status == SQAReport.Status.ERROR:
            text = mooseutils.colorText('FAIL', 'LIGHT_RED', colored=self.color_text)
        elif status == SQAReport.Status.WARNING:
            text = mooseutils.colorText('WARNING', 'LIGHT_YELLOW', colored=self.color_text)
        else:
            text = mooseutils.colorText('OK', 'LIGHT_GREEN', colored=self.color_text)
        return text

    @staticmethod
    def _getFiles(locations):
        """Get complete list of files for the given *locations*."""
        file_list = list()
        for working_dir in locations:
            path = mooseutils.eval_path(working_dir)
            if mooseutils.git_is_repo(path):
                file_list += mooseutils.git_ls_files(path)
            else:
                file_list += glob.glob(os.path.join(path,'**', '*.*'), recursive=True)
        return file_list
