#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re
from .exceptions import MooseDocsException

def regex(pattern, content, flags=None):
    """
    Tool for searching for "content" within a regex and raising the desired exception if not found.
    """
    match = re.search(pattern, content, flags=flags)
    if match:
        if 'content' in match.groupdict():
            content = match.group('content')
        elif 'remove' in match.groupdict():
            content = content[:match.start('remove')] + content[match.end('remove'):]
        else:
            content = match.group()
    else:
        msg = "Failed to match regular expression: {}"
        raise MooseDocsException(msg, pattern)

    return content
