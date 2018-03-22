#!/usr/bin/env python2
import unittest

from MooseDocs.tree import html

class TestHTML(unittest.TestCase):
    """
    Tests for html tree structure.
    """
    def testTag(self):
        tag = html.Tag(None, 'section')
        self.assertEqual(tag.write(), '<section></section>')

    def testString(self):
        tag = html.String(content=u'section')
        self.assertEqual(tag.write(), 'section')

        tag = html.String(content=u'<section>', escape=True)
        self.assertEqual(tag.write(), '&lt;section&gt;')

    def testTagString(self):
        tag = html.Tag(None, 'h1')
        html.String(content=u'foo', parent=tag)
        self.assertEqual(tag.write(), '<h1>foo</h1>')

        with self.assertRaises(TypeError) as e:
            html.String(parent=html.String())
        gold = "If set, the parent of he html.String 'String' must be a html.Tag object, a " \
               "'String'  was provided."
        self.assertEqual(e.exception.message, gold)


if __name__ == '__main__':
    unittest.main(verbosity=2)
