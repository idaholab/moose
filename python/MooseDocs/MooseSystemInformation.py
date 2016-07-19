import logging
import os
from MooseInformationBase import MooseInformationBase
from MarkdownTable import MarkdownTable
from MooseObjectParameterTable import MooseObjectParameterTable

log = logging.getLogger(__name__)

class MooseSystemInformation(MooseInformationBase):
    """
    Object for generating documentation for a MOOSE system.

    Args:
        node: The YAML node for this object.
        syntax: The MooseApplicationSyntax object containing the syntax.
    """

    def __init__(self, node, syntax, **kwargs):
        MooseInformationBase.__init__(self, node, **kwargs)
        self._syntax = syntax

    @staticmethod
    def filename(name):
        """
        The filename for the system documentation.
        """
        return '{}/Overview.md'.format(name.strip('/').replace('/*', '').replace('/<type>', ''))

    def markdown(self):
        """
        Generate the markdown for the system.
        """

        md = []

        # The details
        if os.path.exists(self._details):
            md += ['{{!{}!}}'.format(self._details)]
            md += ['']
        else:
            log.error('Details file does not exist: {}'.format(self._details))

        # Generate table of action parameters
        if self._yaml['parameters']:
            table = MooseObjectParameterTable()
            for param in self._yaml['parameters']:
                table.addParam(param)

            md += ['## Input Parameters']
            md += [table.markdown()]
            md += ['']

        # Generate table of object within the system
        if self._yaml['subblocks']:
            table = MarkdownTable('Name', 'Description')
            for child in self._yaml['subblocks']:

                name = child['name']
                if name.endswith('*') or name.endswith('<type>'):
                    continue

                name = name.split('/')[-1].strip()
                if self._syntax.hasObject(name):
                    name = '[{0}]({0}.md)'.format(name)
                    desc = child['description'].strip()
                    table.addRow(name, desc)

            if table.size() > 0:
                md += ['## Available Objects']
                md += [table.markdown()]
                md += ['']

        return '\n'.join(md)
