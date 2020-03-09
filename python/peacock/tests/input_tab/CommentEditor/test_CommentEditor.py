#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Input.CommentEditor import CommentEditor
from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.comments_changed = 0

    def commentsChanged(self):
        self.comments_changed += 1

    def testComments(self):
        c = "initial comments"
        e = CommentEditor(c)
        e.textChanged.connect(self.commentsChanged)
        self.assertEqual(e.getComments(), c)
        c = "new comments"
        e.setComments(c)
        self.assertEqual(e.getComments(), c)
        self.assertEqual(self.comments_changed, 1)

if __name__ == '__main__':
    Testing.run_tests()
