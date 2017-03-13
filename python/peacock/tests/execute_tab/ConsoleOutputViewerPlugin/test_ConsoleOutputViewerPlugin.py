#!/usr/bin/env python
from peacock.Execute.ConsoleOutputViewerPlugin import ConsoleOutputViewerPlugin
from peacock.utils import Testing

class Tests(Testing.PeacockTester):
    def setUp(self):
        super(Tests, self).setUp()

    def createWidget(self):
        w = ConsoleOutputViewerPlugin()
        w.show()
        w.setEnabled(True)
        return w

    def testOutput(self):
        w = self.createWidget()
        w.onOutputAdded("foo")
        self.assertEqual(w.toPlainText(), "foo")
        self.assertTrue(w.do_scroll)
        w.onOutputAdded('<span style="color:red;">bar</span>')
        self.assertEqual(w.toPlainText(), "foo\nbar")
        self.assertTrue(w.do_scroll)

        # fill up the screen with text so that we can test the auto scrolling
        for i in xrange(1000):
            w.onOutputAdded("foo\n")
        # auto scrolling should be on and we are at the end
        self.assertTrue(w.do_scroll)
        self.assertEqual(w.vert_bar.value(), w.vert_bar.maximum())
        # simulate user changing the view
        w.vert_bar.setValue(100)
        self.assertFalse(w.do_scroll)
        # add more text, we shouldn't scroll
        for i in xrange(100):
            w.onOutputAdded("foo\n")
        self.assertEqual(w.vert_bar.value(), 100)

        # scroll back to the bottom
        w.vert_bar.setValue(w.vert_bar.maximum())
        self.assertTrue(w.do_scroll)
        for i in xrange(100):
            w.onOutputAdded("foo\n")
        self.assertTrue(w.do_scroll)
        self.assertEqual(w.vert_bar.value(), w.vert_bar.maximum())

if __name__ == '__main__':
    Testing.run_tests()
