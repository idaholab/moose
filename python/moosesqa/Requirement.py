#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import mooseutils

@mooseutils.addProperty('name', ptype=str)
@mooseutils.addProperty('path', ptype=str)
@mooseutils.addProperty('filename', ptype=str)
@mooseutils.addProperty('line', ptype=int)
@mooseutils.addProperty('label', ptype=str)
@mooseutils.addProperty('requirement', ptype=str)
@mooseutils.addProperty('requirement_line', ptype=int)
@mooseutils.addProperty('design', ptype=list)
@mooseutils.addProperty('design_line', ptype=int)
@mooseutils.addProperty('issues', ptype=list)
@mooseutils.addProperty('issues_line', ptype=int)
@mooseutils.addProperty('details', ptype=list)
@mooseutils.addProperty('detail', ptype=str)
@mooseutils.addProperty('detail_line', ptype=int)
@mooseutils.addProperty('deprecated', ptype=bool, default=False)
@mooseutils.addProperty('deprecated_line', ptype=int)
@mooseutils.addProperty('duplicate', ptype=bool, default=False)
@mooseutils.addProperty('verification', ptype=list)
@mooseutils.addProperty('verification_line', ptype=int)
@mooseutils.addProperty('validation', ptype=list)
@mooseutils.addProperty('validation_line', ptype=int)
@mooseutils.addProperty('prerequisites', ptype=set)
@mooseutils.addProperty('skip', ptype=bool, default=False)
@mooseutils.addProperty('deleted', ptype=bool, default=False)
class Requirement(mooseutils.AutoPropertyMixin):
    """struct for storing Requirement information."""

    def __init__(self, *args, **kwargs):
        kwargs.setdefault('details', list())
        kwargs.setdefault('prerequisites', set())
        super().__init__(*args, **kwargs)

    @property
    def testable(self):
        out = [self.skip, self.deleted]
        for detail in self.details:
            out += [detail.skip, detail.deleted]
        return not any([v for v in out])

    def __str__(self):
        frmt = '{}:\n  Requirement: {}\n       Design: {}\n       Issues: {}'
        return frmt.format(self.name, self.requirement, repr(self.design), repr(self.issues))
