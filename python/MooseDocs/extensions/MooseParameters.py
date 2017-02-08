import os
import re
import collections
import logging
log = logging.getLogger(__name__)

from markdown.inlinepatterns import Pattern
from markdown.util import etree
from MooseSyntaxBase import MooseSyntaxBase
from MooseObjectParameterTable import MooseObjectParameterTable

class MooseParameters(MooseSyntaxBase):
    """
    Creates parameter tables for Actions and MooseObjects.
    """

    RE = r'^!parameters\s+(.*?)(?:$|\s+)(.*)'

    def __init__(self, **kwargs):
        super(MooseParameters, self).__init__(self.RE, **kwargs)
        self._settings['display'] = "collapsible"
        self._settings['title'] = 'Input Parameters'
        self._settings['title_level'] = 2

    def handleMatch(self, match):
        """
        Return table(s) of input parameters.
        """

        # Extract Syntax and Settings
        syntax = match.group(2)
        settings = self.getSettings(match.group(3))

        # Locate description
        info = self.getInfo(syntax)
        if not info:
            return self.createErrorElement('Failed to locate MooseObject or Action for the command: !parameters {}'.format(syntax))

        # Create the tables (generate 'Required' and 'Optional' initially so that they come out in the proper order)
        tables = collections.OrderedDict()
        tables['Required'] = MooseObjectParameterTable(display_type = settings['display'])
        tables['Optional'] = MooseObjectParameterTable(display_type = settings['display'])

        # Loop through the parameters in yaml object
        for param in info.parameters or []:
            name = param['group_name']
            if not name and param['required']:
                name = 'Required'
            elif not name and not param['required']:
                name = 'Optional'

            if name not in tables:
                tables[name] = MooseObjectParameterTable(display_type = settings['display'])
            tables[name].addParam(param)

        # Produces a debug message if parameters are empty, but generally we just want to include the
        # !parameters command, if parameters exist then a table is produce otherwise nothing happens. This
        # will allow for parameters to be added and the table appear if it was empty.
        if not any(tables.values()):
            log.debug('Unable to locate parameters for {}'.format(info.name))
        else:
            el = self.applyElementSettings(etree.Element('div'), settings)
            if settings['title']:
                title = etree.SubElement(el, 'h{}'.format(str(settings['title_level'])))
                title.text = settings['title']
            for key, table in tables.iteritems():
                if table:
                    subtitle = etree.SubElement(el, 'h{}'.format(str(settings['title_level'] + 1)))
                    subtitle.text = '{} {}'.format(key, 'Parameters')
                    el.append(table.html())
            return el
