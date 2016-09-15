import re
import os
import MooseDocs
from MooseTextPatternBase import MooseTextPatternBase

class MooseTextFile(MooseTextPatternBase):
    """
    A markdown extension for including complete source code files.
    """
    RE = r'^!text\s+(.*?)(?:$|\s+)(.*)'

    def __init__(self, **kwargs):
        super(MooseTextFile, self).__init__(self.RE, language='text', **kwargs)

        # Add to the default settings
        self._settings['line'] =  None
        self._settings['start'] =  None
        self._settings['end'] =  None

    def handleMatch(self, match):
        """
        Process the text file provided.
        """

        # Update the settings from regex match
        settings, styles = self.getSettings(match.group(3))

        # Read the file
        rel_filename = match.group(2).lstrip('/')
        filename = self.checkFilename(rel_filename)

        if not filename:
            return self.createErrorElement(rel_filename)

        if settings['line']:
            content = self.extractLine(filename, settings["line"])

        elif settings['start'] or settings['end']:
            content = self.extractLineRange(filename, settings['start'], settings['end'])

        else:
            with open(filename) as fid:
                content = fid.read()

        if content == None:
            log.error("Failed to extract content from {}.".format(filename))
            return self.createErrorElement(rel_filename, 'No content')

        # Return the Element object
        el = self.createElement(match.group(2), content, filename, rel_filename, settings, styles)
        return el

    def extractLine(self, filename, desired):
        """
        Function for returning a single line.

        Args:
            desired[str]: The text to look for within the source file.
        """

        # Read the lines
        with open(filename) as fid:
            lines = fid.readlines()

        # Search the lines
        content = None
        for line in lines:
            if desired in line:
                content = line

        return content

    def extractLineRange(self, filename, start, end):
        """
        Function for extracting content between start/end strings.

        Args:
            start[str|None]: The starting line (when None is provided the beginning is used).
            end[str|None]: The ending line (when None is provided the end is used).
        """

        # Read the lines
        with open(filename) as fid:
            lines = fid.readlines()

        start_idx = None
        end_idx = None

        # Search the lines
        content = None
        for i in range(len(lines)):
            if start != None and start in lines[i]:
                start_idx = i
            if end != None and end in lines[i]:
                end_idx = i

        if end == None:
            return ''.join(lines[start_idx:])
        elif start == None:
            return ''.join(lines[:end_idx])
        else:
            return ''.join(lines[start_idx:end_idx])
