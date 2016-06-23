import logging
log = logging.getLogger(__name__)

class MarkdownTable(object):
    """
    A generic tool for generating Markdown tables.

    Args:
        column_headers[list]: A list of strings that contain the column headers.
    """

    def __init__(self, *args):
        self._column_headers = args
        self._column_widths = []
        self._rows = []

        for col in self._column_headers:
            self._column_widths.append(len(col))

    def size(self):
        """
        Return the number of rows.
        """
        return len(self._rows)


    def addRow(self, *args):
        """
        Add a row to the table.

        Args:
            *args: The items to include in the table (must be the same length as the supplied headers).
        """

        if len(args) != len(self._column_headers):
            msg = "The number of supplied items ({}) does not match the number of columns ({}).".format(len(args), len(self._column_headers))
            raise Exception(msg)

        for i in range(len(self._column_headers)):
            self._column_widths[i] = max(self._column_widths[i], len(args[i]))

        self._rows.append(args)

    def markdown(self):
        """
        Return the parameter table in markdown format. (public)
        """

        md = []

        s = self._buildFormatString(self._column_headers)

        frmt = '| ' + ' | '.join( ['{:<{}s}'] * (len(s)/2) ) + ' |'
        md += [frmt.format(*s)]

        md += ['']
        for i in range(len(self._column_headers)):
            md[-1] += '| ' + '-'*self._column_widths[i] + ' '
        md[-1] += '|'

        for row in self._rows:
            text = []
            for item in row:
                text.append(item)
            s = self._buildFormatString(text)
            md += [frmt.format(*s)]

        return '\n'.join(md)

    def _buildFormatString(self, text):
        """
        A helper method for building format strings. (protected)
        """
        output = []
        for i in range(len(text)):
            output.append(text[i].strip())
            output.append(self._column_widths[i])
        return output
