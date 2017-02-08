import os
import re

from markdown.inlinepatterns import Pattern
from markdown.util import etree
from MooseSyntaxBase import MooseSyntaxBase

class MooseDescription(MooseSyntaxBase):
    """
    Creates parameter tables for Actions and MooseObjects.
    """

    RE = r'^!description\s+(.*?)(?:$|\s+)(.*)'

    def __init__(self, **kwargs):
        super(MooseDescription, self).__init__(self.RE, **kwargs)

    def handleMatch(self, match):
        """
        Return the class description html element.
        """

        # Extract Syntax and Settings
        syntax = match.group(2)
        settings = self.getSettings(match.group(3))

        # Locate description
        info = self.getInfo(syntax)
        if not info:
            return self.createErrorElement('Failed to locate MooseObject or Action for the command: !description {}'.format(syntax))

        # Create an Error element, but do not produce warning/error log because the
        # moosedocs check/generate commands produce errors.
        if info.description == None:
            return self.createErrorElement('Failed to locate class description for {} syntax.'.format(info.name), warning=None)

        # Create the html element with supplied styles
        el = self.applyElementSettings(etree.Element('p'), settings)
        el.text = info.description
        return el
