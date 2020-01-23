#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import mooseutils
from .box import box
def report_error(message, filename, line=None, src='', traceback=None, prefix='ERROR'):
    """
    Helper for reporting error to logging module.

    Inputs:
        message[str]: The message to report, ERROR is automatically appended
        page[pages.Page]: The page that created the error
        info[Information]: The lexer information object
        traceback: The traceback (should be a string from traceback.format_exc())
    """
    title = '{}: {}'.format(prefix, message)
    if src:
        src = mooseutils.colorText(box(src, line=line, width=100), 'LIGHT_CYAN')

    if line is not None:
        filename = mooseutils.colorText('{}:{}\n'.format(filename, line), 'RESET')
    else:
        filename = mooseutils.colorText('{}\n'.format(filename), 'RESET')

    trace = '\n' + mooseutils.colorText(traceback, 'GREY') if traceback else ''
    return '\n{}\n{}{}{}\n'.format(title, filename, src, trace)
