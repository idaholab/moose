#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

import re
import logging
LOG = logging.getLogger(__name__)

def slugify(text, *args):
    r"""
    A utility for converting text into acceptable filename.

    Spaces are converted to underscore.
    :/\.,[]{}()!"'*?<>| produce an error, unless a conversion pair is supplied.

    Returns a tuple of the replace string and a flag, if True the returned string is valid and
    does not contain invalid characters.
    """

    # Build replacement map
    conv = {' ':'_'}
    for key, value in args:
        conv[key] = value

    # Loop through all matches and replace
    valid = True
    regex = re.compile(r'(?P<char>[\s:\/\\\.\,\[\]\{\}\(\)\!\"\'\*\?\<\>\|])')
    for match in regex.finditer(text):
        char = match.group('char')
        if char in conv:
            text = text.replace(char, conv[char])
        else:
            valid = False

    return text, valid
