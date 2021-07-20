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

class TestSpecification(object):
    """Object for storing Test specification information w/r/t SQA"""

    def __init__(self, *args, **kwargs):
        self.name = kwargs.pop('name', None)
        self.filename = kwargs.pop('filename', None)
        self.local = kwargs.pop('local', None)
        self.line = kwargs.pop('line', None)
        self.type = kwargs.pop('type', None)
        self.prerequisites = kwargs.pop('prerequisites', set())
        self.skip = kwargs.pop('skip', False)
        self.deleted = kwargs.pop('deleted', False)
        self.text = kwargs.pop('text', None)

    @property
    def testable(self):
        out = [self.skip, self.deleted]
        return not any([v for v in out])

    def __str__(self):
        return 'Specification: {}'.format(self.name)

class Requirement(object):
    """struct for storing Requirement information."""

    def __init__(self, *args, **kwargs):
        self.name = kwargs.pop('name', None)
        self.filename = kwargs.pop('filename', None)
        self.line = kwargs.pop('line', None)
        self.specification = kwargs.pop('specification', None)
        self.details = kwargs.pop('details', list())
        self.requirement = kwargs.pop('requirement', None)
        self.requirement_line = kwargs.pop('requirement_line', None)
        self.issues = kwargs.pop('issues', None)
        self.issues_line = kwargs.pop('issues_line', None)
        self.design = kwargs.pop('design', None)
        self.design_line = kwargs.pop('design_line', None)
        self.collections = kwargs.pop('collections', None)
        self.collections_line = kwargs.pop('collections_line', None)
        self.classification = kwargs.pop('classification', None)
        self.classification_line = kwargs.pop('classification_line', None)
        self.deprecated = kwargs.pop('deprecated', False)
        self.deprecated_line = kwargs.pop('deprecated_line', None)
        self.verification = kwargs.pop('verification', None)
        self.verification_line = kwargs.pop('verification_line', None)
        self.validation = kwargs.pop('validation', None)
        self.validation_line = kwargs.pop('validation_line', None)
        self.label = kwargs.pop('label', None)
        self.duplicate = kwargs.pop('duplicate', False)
        self.prefix = kwargs.pop('prefix', None)

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

class Detail(object):
    """struct for storing Detail information of a Requirement."""

    def __init__(self, *args, **kwargs):
        self.name = kwargs.pop('name', None)
        self.filename = kwargs.pop('filename', None)
        self.line = kwargs.pop('line', None)
        self.detail = kwargs.pop('detail', None)
        self.detail_line = kwargs.pop('detail_line', None)
        self.specification = kwargs.pop('specification', None)
        self.duplicate = kwargs.pop('duplicate', False)

    @property
    def testable(self):
        if self.specification is not None:
            out = [self.specification.skip, self.specification.deleted]
            return not any([v for v in out])
        return False

    def __str__(self):
        return 'Detail: {}; specification = {}'.format(self.name, self.specification.name)
