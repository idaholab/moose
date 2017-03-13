#!/usr/bin/env python
from peacock.Input.CommentEditor import CommentEditor
from peacock.utils import Testing

class Tests(Testing.PeacockTester):
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
