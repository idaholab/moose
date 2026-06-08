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
import logging
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, floats, bibtex
from MooseDocs import common

logging.basicConfig()


class TestBibtexCite(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, bibtex]
    TEXT = "[!cite](slaughter2014framework, gaston2015physics)\n\n!bibtex bibliography"

    def setupContent(self):
        config = [
            dict(
                root_dir="framework/doc/content", content=["bib/moose.bib"]
            )
        ]
        return common.get_content(config, ".bib")

    def testAuthorYearHTML(self):
        _, res = self.execute(self.TEXT)
        # Inline citation is rendered as author-year links.
        p = res(0)
        self.assertHTMLTag(p, "p")
        self.assertHTMLTag(p(0), "a", string="Slaughter et al. (2014)")
        self.assertHTMLString(p(1), " and ")
        self.assertHTMLTag(p(2), "a", string="Gaston et al. (2015)")


class TestBibtexNumber(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, bibtex]
    TEXT = "[!cite](slaughter2014framework, gaston2015physics)\n\n!bibtex bibliography"

    def setupExtension(self, ext):
        if ext == bibtex:
            return dict(citation_style="number")

    def setupContent(self):
        config = [
            dict(
                root_dir="framework/doc/content", content=["bib/moose.bib"]
            )
        ]
        return common.get_content(config, ".bib")

    def testNumberHTML(self):
        _, res = self.execute(self.TEXT)
        # Inline citation is rendered as bracketed numbers in citation order.
        p = res(0)
        self.assertHTMLTag(p, "p")
        self.assertHTMLString(p(0), "[")
        self.assertHTMLTag(p(1), "a", string="1", href="#slaughter2014framework")
        self.assertHTMLString(p(2), ", ")
        self.assertHTMLTag(p(3), "a", string="2", href="#gaston2015physics")
        self.assertHTMLString(p(4), "]")

    def testBibliographyOrder(self):
        _, res = self.execute(self.TEXT)
        # The reference list is ordered to match the inline numbering.
        ol = res(2)(0)
        self.assertHTMLTag(ol, "ol")
        self.assertEqual(ol(0)["id"], "slaughter2014framework")
        self.assertEqual(ol(1)["id"], "gaston2015physics")


if __name__ == "__main__":
    unittest.main(verbosity=2)
