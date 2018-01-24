#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring
import logging
from markdown.util import etree

LOG = logging.getLogger(__name__)

class MarkdownTable(object):
    """
    A generic tool for generating Markdown tables.

    Args:
      column_headers[list]: A list of strings that contain the column headers.
    """

    def __init__(self, *args):
        self._column_headers = args
        self._rows = []

    def size(self):
        """
        Return the number of rows.
        """
        return len(self._rows)

    def __nonzero__(self):
        """
        bool operator.
        """
        return self.size() > 0

    def addRow(self, *args):
        """
        Add a row to the table.

        Args:
          *args: Items to include in the table (must be the same length as the supplied headers).
        """

        if len(args) != len(self._column_headers):
            msg = "The number of supplied items ({}) does not match the number of columns ({})."
            raise Exception(msg.format(len(args), len(self._column_headers)))

        self._rows.append(args)

    def html(self):
        """
        Return the table in an html etree object.
        """
        table = etree.Element('table')
        tr = etree.SubElement(table, 'tr')
        for h in self._column_headers:
            th = etree.SubElement(tr, 'th')
            th.text = h
        for row in self._rows:
            tr = etree.SubElement(table, 'tr')
            for d in row:
                td = etree.SubElement(tr, 'td')
                if isinstance(d, str):
                    td.text = d
                else:
                    td.append(d)
        return table
