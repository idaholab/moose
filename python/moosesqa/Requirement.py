#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import enum
import mooseutils

@mooseutils.addProperty('name', ptype=str)
@mooseutils.addProperty('filename', ptype=str)
@mooseutils.addProperty('local', ptype=str)
@mooseutils.addProperty('line', ptype=int)
@mooseutils.addProperty('type', ptype=str)
@mooseutils.addProperty('prerequisites', ptype=set)
@mooseutils.addProperty('skip', ptype=bool, default=False)
@mooseutils.addProperty('deleted', ptype=bool, default=False)
@mooseutils.addProperty('text', ptype=str)
class TestSpecification(mooseutils.AutoPropertyMixin):
    """Object for storing Test specification information w/r/t SQA"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if self.prerequisites is None:
            self.prerequisites = set()

    @property
    def testable(self):
        out = [self.skip, self.deleted]
        return not any([v for v in out])

    def __str__(self):
        return 'Specification: {}'.format(self.name)

@mooseutils.addProperty('name', ptype=str)
@mooseutils.addProperty('filename', ptype=str)
@mooseutils.addProperty('line', ptype=int)
@mooseutils.addProperty('specification', ptype=TestSpecification)
@mooseutils.addProperty('details', ptype=list)
@mooseutils.addProperty('requirement', ptype=str)
@mooseutils.addProperty('requirement_line', ptype=int)
@mooseutils.addProperty('issues', ptype=list)
@mooseutils.addProperty('issues_line', ptype=int)
@mooseutils.addProperty('design', ptype=list)
@mooseutils.addProperty('design_line', ptype=int)
@mooseutils.addProperty('collections', ptype=set)
@mooseutils.addProperty('collections_line', ptype=int)
@mooseutils.addProperty('classification', ptype=str)
@mooseutils.addProperty('classification_line', ptype=int)
@mooseutils.addProperty('deprecated', ptype=bool, default=False)
@mooseutils.addProperty('deprecated_line', ptype=int)
@mooseutils.addProperty('verification', ptype=list)
@mooseutils.addProperty('verification_line', ptype=int)
@mooseutils.addProperty('validation', ptype=list)
@mooseutils.addProperty('validation_line', ptype=int)
@mooseutils.addProperty('label', ptype=str)
@mooseutils.addProperty('duplicate', ptype=bool, default=False)
@mooseutils.addProperty('prefix', ptype=str, default=None)
class Requirement(mooseutils.AutoPropertyMixin):
    """struct for storing Requirement information."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if self.details is None:
            self.details = list()

    @property
    def names(self):
        out = set()
        if self.specification is not None:
            out.add(self.specification.name)

        elif self.details:
            for d in self.details:
                if d.specification is not None:
                    out.add(d.specification.name)
        return out

    @property
    def testable(self):
        if self.specification is not None:
            return self.specification.testable
        elif self.details:
            return all([d.specification.testable for d in self.details if d.specification is not None])
        return False

    @property
    def prerequisites(self):
        prereq = set()
        if self.specification is not None:
            prereq.update(self.specification.prerequisites)
        elif self.details:
            for d in self.details:
                if d.specification is not None:
                    prereq.update(d.specification.prerequisites)
        return prereq

    @property
    def types(self):
        test_types = set()
        if self.specification is not None:
            test_types.add(self.specification.type)
        elif self.details:
            for d in self.details:
                if d.specification is not None:
                    test_types.add(d.specification.type)
        return {t for t in test_types if t is not None} or None

    @property
    def specifications(self):
        if self.specification is not None:
            yield self.specification
        elif self.details:
            for d in self.details:
                if d.specification is not None:
                    yield d.specification

    def __str__(self):
        out = 'Requirement: {}; requirement = {}; design = {}; issues = {}'

        if self.specification:
            out += '; specification = ' + str(self.specification)
        elif self.details:
            for d in self.details:
                out += '\n  ' + str(d)
        return out.format(self.name, repr(self.requirement), repr(self.design), repr(self.issues))

@mooseutils.addProperty('name', ptype=str)
@mooseutils.addProperty('filename', ptype=str)
@mooseutils.addProperty('line', ptype=int)
@mooseutils.addProperty('detail', ptype=str)
@mooseutils.addProperty('detail_line', ptype=int)
@mooseutils.addProperty('specification', ptype=TestSpecification)
@mooseutils.addProperty('duplicate', ptype=bool, default=False)
class Detail(mooseutils.AutoPropertyMixin):
    """struct for storing Detail information of a Requirement."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    @property
    def testable(self):
        if self.specification is not None:
            out = [self.specification.skip, self.specification.deleted]
            return not any([v for v in out])
        return False

    def __str__(self):
        return 'Detail: {}; specification = {}'.format(self.name, self.specification.name)
