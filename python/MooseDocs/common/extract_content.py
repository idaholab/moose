#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import copy
import re
from .regex import regex
from .exceptions import MooseDocsException
from .parse_settings import get_settings_as_dict


def extractContentSettings():
    """Settings for extractContent function"""
    settings = dict()
    settings['prepend'] = (None, "Text to include prior to each line of the included text.")
    settings['append'] = ('', "Text to include after each line of the included text.")
    settings['header'] = (None, "Text to include prior to the included text.")
    settings['header-newlines'] = (1, "The number of newlines after the header.")
    settings['footer'] = ('', "Text to include after the included text.")
    settings['footer-newlines'] = (1, "The number of newlines before the footer.")
    settings['indent'] = (0, "The level of indenting to apply to the included text.")
    settings['strip-header'] = (True, "When True the MOOSE header is removed for display.")
    settings['fix-moose-header'] = (True, "In C/h files within MOOSE the '//*' is used for the "
                                          "header at the top. This breaks the highlighting, this "
                                          "option removes these and replaces them with '//'.")
    settings['strip-extra-newlines'] = (True, "Removes extraneous new lines from the text.")
    settings['strip-leading-whitespace'] = (False, "When True leading white-space is removed "
                                            "from the included text.")
    settings['line'] = (None, "A portion of text that unique identifies a single line to "
                        "include.")
    settings['re'] = (None, "Extract content via a regex, if the 'content' group exists it " \
                      "is used as the desired content; if 'remove' group exists it is extracted; otherwise group 0 is used for the content.")
    settings['re-flags'] = ('re.M|re.S|re.U', "Python re flags.")
    settings['start'] = (None, "A portion of text that unique identifies the starting "
                         "location for including text, if not provided the beginning "
                         "of the file is utilized.")
    settings['end'] = (None, "A portion of text that unique identifies the ending location "
                       "for including text, if not provided the end of the file is "
                       "used. By default this line is not included in the display.")
    settings['include-start'] = (True, "When False the text captured by the 'start' setting "
                                 "is excluded in the displayed text.")
    settings['include-end'] = (False, "When True the text captured by the 'end' setting is "
                               "included in the displayed text.")
    settings['replace'] = (None, "List of replacement string pairs: ['foo','bar', 'boom','baam'] " \
                                 "replaces 'foo' with 'bar' and 'boom' with 'baam'.")
    return settings


def extractContent(content, settings):
    """
    Extract the desired content from the supplied raw text from a file.

    Inputs:
        filename[str]: The file to read (known to exist already).
        settings[dict]: The setting from the createToken method.
    """
    raw = copy.copy(content) # complete copy of original
    if settings['re'] is not None:
        content = regex(settings['re'], content, eval(settings['re-flags']))

    elif settings['line'] is not None:
        content = extractLine(content, settings["line"])

    elif (settings['start'] is not None) or (settings['end'] is not None):
        content = extractLineRange(content,
                                   settings['start'],
                                   settings['end'],
                                   settings['include-start'],
                                   settings['include-end'])

    content = prepareContent(content, settings)

    # Locate the line
    line = 1
    match = re.search(r'(.*?)$', content, flags=re.MULTILINE)
    if match is not None:
        first = match.group(1)
        for i, raw_line in enumerate(raw.splitlines()):
            if first in raw_line:
                line = i
                continue
    return content, line

def prepareContent(content, settings):
    """
    Apply the various filters and adjustment to the supplied text.

    Inputs:
        content[str]: The extracted content.
        settings[dict]: The setting from the createToken method.
    """
    # Strip leading/trailing newlines
    content = re.sub(r'^(\n*)', '', content)
    content = re.sub(r'(\n*)$', '', content)

    # Strip extra new lines (optional)
    if settings['strip-extra-newlines']:
        content = re.sub(r'(\n{3,})', '\n\n', content)

    # Strip header
    if settings['strip-header']:
        content = re.sub(r'^((#\*)|(\/{2}\*)).*?\n', '', content, flags=re.MULTILINE)

    # Strip leading/trailing white-space
    if settings['strip-leading-whitespace']:
        content = re.sub(r'^(\s+)', '', content, flags=re.MULTILINE)

    # Add indent
    if settings['indent'] > 0:
        replace = r'{}\1'.format(' '*int(settings['indent']))
        content = re.sub(r'^(.*?)$', replace, content, flags=re.MULTILINE|re.UNICODE)

    # prepend/append
    if settings['prepend'] is not None:
        replace = r'{}\1'.format(settings['prepend'])
        content = re.sub(r'^(.*?)$', replace, content, flags=re.MULTILINE|re.UNICODE)

    if settings['append'] is not None:
        replace = r'\1{}'.format(settings['append'])
        content = re.sub(r'^(.*?)$', replace, content, flags=re.MULTILINE|re.UNICODE)

    if settings['header'] is not None:
        content = '{}{}{}'.format(settings['header'], '\n'*settings['header-newlines'], content)

    if settings['footer'] is not None:
        content = '{}{}{}'.format(content, '\n'*settings['footer-newlines'], settings['footer'])

    if settings['fix-moose-header']:
        content = fix_moose_header(content)
        content = re.sub(r'^//\*', '//', content, flags=re.MULTILINE|re.UNICODE)

    if settings['replace']:
        pairs = eval(settings['replace'])
        if len(pairs) % 2:
            msg = "The 'replace' input must be a list of replacement string pairs."
            raise MooseDocsException(msg)
        for f, r in [(pairs[i], pairs[i+1]) for i in range(0, len(pairs), 2)]:
            content = content.replace(f, r)

    return content

def fix_moose_header(content):
    """
    Fixes up MOOSE CPP headers so they are highlighted correctly on the website.

    Input:
        content[str]: The source code to modify.
    """
    return re.sub(r'^//\*', '//', content, flags=re.MULTILINE|re.UNICODE)

def extractLine(content, desired):
    """
    Function for returning a single line.

    Args:
      conetnt[str]: The string content to examine.
      desired[str]: The text to look for within the source file.
    """

    lines = content.split('\n')

    # Search the lines
    content = None
    for line in lines:
        if desired in line:
            content = line

    return content

def extractLineRange(content, start, end, include_start, include_end):
    """
    Function for extracting content between start/end strings.

    Args:
      conetnt[str]: The string content to examine.
      start[str|None]: The starting line (when None is provided the beginning is used).
      end[str|None]: The ending line (when None is provided the end is used).
      include-start[bool]: If True then the start string is included
      include-end[bool]: If True then the end string is included
    """
    lines = content.split('\n')
    start_idx = 0
    end_idx = len(lines)

    if start:
        for i in range(end_idx):
            if start in lines[i]:
                start_idx = i if include_start else i+1
                break
    if end:
        for i in range(start_idx, end_idx):
            if end in lines[i]:
                end_idx = i + 1 if include_end else i
                break

    return '\n'.join(lines[start_idx:end_idx])
