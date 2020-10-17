#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import re

def apply_template_arguments(content, **template_args):
    """
    Helper for applying template args (e.g., {{app}})
    """
    if not isinstance(content, (str, str)):
        return content

    content = re.sub(r'{{(?P<key>.*?)}}', lambda m: _sub(m, template_args), content)
    return content

def _sub(match, template_args):
    key = match.group('key')
    arg = template_args.get(key, None)
    if key is None:
        msg = "The template argument '{}' was not defined in the !template load command."
        raise exceptions.MooseDocsException(msg, key)
    return arg
