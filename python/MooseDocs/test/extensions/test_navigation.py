#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from unittest.mock import MagicMock
from MooseDocs.extensions import navigation, content


class TestSourceLinkURL(unittest.TestCase):
    """
    Unit tests for NavigationExtension._sourceLinkURL, the fallback used by
    the breadcrumb code when a parent directory has no index.md. It looks up
    the directory name in the content extension's source_links mapping and
    returns a URL relative to the current page (or None to indicate that the
    caller should render a non-clickable <span>).
    """

    def _makeNav(self, source_links, target=None):
        """Build a NavigationExtension with a stubbed translator."""
        nav = navigation.NavigationExtension.__new__(navigation.NavigationExtension)

        content_ext = MagicMock(spec=content.ContentExtension)
        content_ext.get.return_value = source_links

        translator = MagicMock()
        translator.extensions = [content_ext]
        translator.findPage.return_value = target
        nav.setTranslator(translator)
        return nav

    def testHit(self):
        target = MagicMock()
        target.relativeDestination.return_value = "folder.html"
        nav = self._makeNav({"folder": "extensions/folder/folder.md"}, target=target)

        page = MagicMock()
        self.assertEqual(nav._sourceLinkURL("folder", page), "folder.html")
        target.relativeDestination.assert_called_once_with(page)

    def testNameNotInMapping(self):
        nav = self._makeNav({"folder": "extensions/folder/folder.md"}, target=None)
        self.assertIsNone(nav._sourceLinkURL("not_configured", MagicMock()))

    def testMappingPresentButTargetMissing(self):
        # findPage returns None (page not in content set)
        nav = self._makeNav({"folder": "missing.md"}, target=None)
        self.assertIsNone(nav._sourceLinkURL("folder", MagicMock()))

    def testNoContentExtensionLoaded(self):
        nav = navigation.NavigationExtension.__new__(navigation.NavigationExtension)
        translator = MagicMock()
        translator.extensions = []  # no ContentExtension loaded
        nav.setTranslator(translator)
        self.assertIsNone(nav._sourceLinkURL("folder", MagicMock()))


if __name__ == "__main__":
    unittest.main(verbosity=2)
