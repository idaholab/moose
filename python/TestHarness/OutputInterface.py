#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os

class OutputInterface:
    """ Helper class for writing output to either memory or a file """
    def __init__(self):
        # The in-memory output, if any
        self.output = ''
        # The path to write output to, if any
        self.separate_output_path = None

    def setSeparateOutputPath(self, separate_output_path):
        """ Sets the path for writing output to """
        self.separate_output_path = separate_output_path

        # If we have any dangling output, write it
        if self.output:
            self.setOutput(self.output)
            self.output = ''

    def getSeparateOutputFilePath(self):
        """ Gets the path that this output is writing to, if any """
        return self.separate_output_path

    def hasOutput(self):
        """ Whether or not this object has any content written """
        if self.separate_output_path:
            return os.path.isfile(self.separate_output_path)
        return len(self.output)

    def getOutput(self):
        """ Gets the underlying output, either from file or memory """
        if self.separate_output_path:
            try:
                return open(self.separate_output_path, 'r').read()
            except FileNotFoundError:
                pass
        else:
            return self.output
        return ''

    def setOutput(self, output):
        """ Sets the output given some output string """
        if self.separate_output_path:
            open(self.separate_output_path, 'w').write(output)
        else:
            self.output = output

    def appendOutput(self, output):
        """ Appends to the output """
        if self.separate_output_path:
            open(self.separate_output_path, 'a').write(output)
        else:
            self.output += output

    def clearOutput(self):
        """ Clears the output """
        if self.separate_output_path:
            open(self.separate_output_path, 'w').close()
        else:
            self.output = ''
