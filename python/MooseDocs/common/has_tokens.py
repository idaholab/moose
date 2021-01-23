#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import moosetree
def has_tokens(root, *names):
    """
    Return True if the supplied tree of Token objects in *root* contain any of the supplied *names*.
    """
    return moosetree.find(root, func=lambda n: n.name in names) is not None
