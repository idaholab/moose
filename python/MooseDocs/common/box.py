#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import mooseutils

def box(content, title=None, line=None, width=None, color='RESET'):
    """Tool for building unicode box around text, this is used for error reporting."""

    lines = content.split('\n')
    n_lines = len(max(lines, key=len))
    out = ''
    if title:
        out += title + '\n'

    if line is not None:
        num_digits = len(str(line + len(lines)))
        if width:
            n_lines = max([width - num_digits - 2, n_lines])
        out += '{0:>{1}}{2}{3}{4}'.format(' ', num_digits, '\u250C', '\u2500'*n_lines, '\u2510')
        for i, x in enumerate(lines):
            out += '\n{0:>{1}}{2}{3:<{4}}{2}'.format(line+i, num_digits, '\u2502', x, n_lines)
        out += '\n{0:>{1}}{2}{3}{4}'.format(' ', num_digits, '\u2514', '\u2500'*n_lines,
                                             '\u2518')
    else:
        if width:
            n_lines = max([width - 2, n_lines])
        out += '{}{}{}'.format('\u250C', '\u2500'*n_lines, '\u2510')
        for i, x in enumerate(lines):
            out += '\n{0}{1:<{2}}{0}'.format('\u2502', x, n_lines)
        out += '\n{}{}{}'.format('\u2514', '\u2500'*n_lines, '\u2518')

    if color is None:
        return out

    return mooseutils.colorText(out, color)
