# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html


def find_heading(page, id_=None):
    """Helper for returning a copy of the heading tokens."""
    return page.get("title") if id_ is None else page.get("headings", dict()).get(id_)
