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
