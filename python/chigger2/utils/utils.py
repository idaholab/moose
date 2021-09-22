#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
def get_current_window():
    """Return the current Window object (last to instantiate)"""
    from .. import Window
    return Window.__CHIGGER_CURRENT__

def get_current_viewport():
    """Return the current Viewport object (last to instantiate)"""
    from .. import Viewport
    return Viewport.__CHIGGER_CURRENT__

def get_current_exodus_reader():
    """Return the current ExodusReader (last to instantiate)"""
    from ..exodus import ExodusReader
    return ExodusReader.__CHIGGER_CURRENT__
