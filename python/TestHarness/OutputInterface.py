#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import json

class OutputInterface:
    """ Helper class for writing output to either memory or a file """
    def __init__(self):
        # The in-memory output, if any
        self.output = ''
        # The path to write output to, if any
        self.separate_output_path = None

    class BadOutputException(Exception):
        """ Exception that is thrown when bad output is detected """
        def __init__(self, errors):
            self.errors = errors
            message = 'Bad output detected: ' + ', '.join(errors)
            super().__init__(message)

    def setSeparateOutputPath(self, separate_output_path):
        """ Sets the path for writing output to """
        self.separate_output_path = separate_output_path

        # If we have any dangling output, write it
        if self.output:
            self.setOutput(self.output)
            self.output = ''

    def getSeparateOutputFilePath(self) -> str:
        """ Gets the path that this output is writing to, if any """
        return self.separate_output_path

    def hasOutput(self) -> bool:
        """ Whether or not this object has any content written """
        if self.separate_output_path:
            return os.path.isfile(self.separate_output_path)
        return len(self.output) > 0

    def getOutput(self, sanitize: bool = True) -> str:
        """
        Gets the underlying output, either from file or memory

        The sanitize parameter triggers whether or not to check
        for bad output, in which case an exception will be thrown
        if it is found. The intention here is to sanitize it
        ahead of time with self.sanitizeOutput() so that you can
        clean it then and appropriately report the error earlier
        on before the output is used.
        """
        output = ''
        if self.separate_output_path:
            try:
                output = open(self.separate_output_path, 'r').read()
            except FileNotFoundError:
                pass
        else:
            output = self.output

        if sanitize:
            _, sanitize_failures = self._sanitizeOutput(output)
            if sanitize_failures:
                raise self.BadOutputException(sanitize_failures)

        return output

    def setOutput(self, output: str):
        """ Sets the output given some output string """
        if not output:
            return
        if self.separate_output_path:
            open(self.separate_output_path, 'w').write(output)
        else:
            self.output = output

    def appendOutput(self, output: str):
        """ Appends to the output """
        if not output:
            return
        if self.separate_output_path:
            open(self.separate_output_path, 'a').write(output)
        else:
            self.output += output

    def clearOutput(self):
        """ Clears the output """
        if self.separate_output_path:
            if os.path.exists(self.separate_output_path):
                os.remove(self.separate_output_path)
        else:
            self.output = ''

    @staticmethod
    def _sanitizeOutput(output):
        """
        Internal method for taking an output string, sanitizing
        it if needed, and then returning a list of the failures
        that were encountered (if any)
        """
        failures = set()

        # Check for invalid characters
        try:
            json.dumps(output)
        except UnicodeDecodeError:
            # Convert invalid output to something json can handle
            output = output.decode('utf-8','replace').encode('ascii', 'replace')
            # Alert the user that output has invalid characters
            failures.add('invalid output characters')

        # Check for NULL characters
        null_chars = ['\0', '\x00']
        for null_char in null_chars:
            if null_char in output:
                output = output.replace(null_char, 'NULL')
                failures.add('NULL output')

        return output, list(failures)

    def sanitizeOutput(self):
        """
        Sanitizes the output in place and returns a list of the
        checks that failed, if any.

        Should be called before processing the output.
        """
        output, failures = self._sanitizeOutput(self.getOutput(sanitize=False))
        if failures:
            self.setOutput(output)
        return failures
