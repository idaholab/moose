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
        return '{}/Overview.moose.md'.format(name.strip('/').replace('/*', '').replace('/<type>', ''))

    def markdown(self):
        """
        Generate the markdown for the system.
        """

        md = []

        # The details
        if not os.path.exists(self._details):
            log.error('Details file does not exist: {}'.format(self._details))
            md += ['\n\n!!! danger "ERROR!"\n{}The details file does not exist: `{}`\n\n'.format(4*' ', self._details)]
        else:
            md += ['{{!{}!}}'.format(self._details)]
        md += ['']

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

            # Create table for sub-blocks (i.e., '*' syntax)
            if any( [child['name'].endswith('*') for child in self._yaml['subblocks'] ]):
                table = self._genTable(self._yaml['subblocks'])
                if table.size() > 0:
                    md += ['## Available Sub-Objects']
                    md += [table.markdown()]
                    md += ['']

            # If the '*' syntax is not used it is possible to have type and sub-systems (e.g., Executioner block)
            else:
                sub_system_table = MarkdownTable('Name', 'Description')
                for child in self._yaml['subblocks']:

                    # Create table for single-block (i.e., <type>)
                    if child['name'].endswith('<type>'):
                        table = self._genTable(child['subblocks'])
                        if table.size() > 0:
                            md += ['## Available Types']
                            md += [table.markdown()]
                            md += ['']
                    else:
                        name = child['name'].split('/')[-1].strip()
                        name = '[{0}]({0}/Overview.moose.md)'.format(name)
                        desc = child['description'].strip()
                        sub_system_table.addRow(name, desc)

                if sub_system_table.size() > 0:
                    md += ['## Available Sub-systems']
                    md += [sub_system_table.markdown()]
                    md += ['']

        return '\n'.join(md)

    def _genTable(self, node):
        """
        Helper method for creating a system summary markdown tables
        """
        table = MarkdownTable('Name', 'Description')
        for child in node:
            name = child['name']
            if name.endswith('*'):
                continue

            name = name.split('/')[-1].strip()
            if self._syntax.hasObject(name):
                name = '[{0}]({0}.moose.md)'.format(name)
                desc = child['description'].strip()
                table.addRow(name, desc)

        return table
