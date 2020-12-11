#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Tools for parsing key value pairs from a raw string (e.g., 'key=value foo=bar')
"""
import re
import copy

from .exceptions import MooseDocsException

SETTINGS_RE = re.compile(r'(?P<key>[^\s\\=]+)=(?P<value>.*?)(?=(?:\s[^\s\\=]+=|$))',
                         flags=re.MULTILINE|re.UNICODE)

def get_settings_as_dict(settings):
    """Return a dict() of the settings without the description."""
    output = dict()
    for key, value in settings.items():
        output[key] = value[0]
    return output

def match_settings(known, raw):
    """
    Parses a raw string for key, value pairs separated by an equal sign.

    Inputs:
        default[dict]: The default values for the known keys.
        raw[str]: The raw string to parse and inject into a dict().
    """
    unknown = dict()

    if not raw:
        return known, unknown

    for match in SETTINGS_RE.finditer(raw.replace('\n', ' ')):

        key = match.group('key').strip()
        value = match.group('value').strip().replace(r'\=', '=')

        if value.lower() == 'true':
            value = True
        elif value.lower() == 'false':
            value = False
        elif value.lower() == 'none':
            value = None
        elif value and all([v.isdigit() for v in value]):
            value = float(value)

        if key in known:
            known[key] = value
        else:
            unknown[key] = value

    return known, unknown

def parse_settings(defaults, local, error_on_unknown=True):
    """
    Method for parsing settings given a dict() of defaults. If options are supplied that do not
    have an item in defaults and exception will be raised.

    Inputs:
        defaults[dict]: Default settings.
        local[str]: A string of settings to be parsed.
        error_on_unknown[bool]: If True through an exception if values are provided that are not
                                in the default list.
    """
    known = dict((k, v[0]) for k, v in copy.deepcopy(defaults).items())
    settings, unknown = match_settings(known, local)
    if error_on_unknown and unknown:
        msg = "The following key, value settings are unknown:"
        for key, value in unknown.items():
            msg += '\n{}{}={}'.format(' '*4, key, repr(value))
        raise MooseDocsException(msg)
    return settings, unknown
