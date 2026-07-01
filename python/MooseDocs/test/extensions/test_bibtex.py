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
import moosetree
from pybtex.database import Entry, Person
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, floats, bibtex
from MooseDocs import base, common

logging.basicConfig()


class TestBibtexCite(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, bibtex]
    TEXT = "[!cite](slaughter2014framework, gaston2015physics)\n\n!bibtex bibliography"

    def setupContent(self):
        config = [dict(root_dir="framework/doc/content", content=["bib/moose.bib"])]
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
        config = [dict(root_dir="framework/doc/content", content=["bib/moose.bib"])]
        return common.get_content(config, ".bib")

    def testNumberHTML(self):
        _, res = self.execute(self.TEXT)
        # Textual (!cite) citations prepend the author to the bracketed number.
        p = res(0)
        self.assertHTMLTag(p, "p")
        self.assertHTMLString(p(0), "Slaughter et al. ")
        self.assertHTMLTag(p(1), "a", string="[1]", href="#slaughter2014framework")
        self.assertHTMLString(p(2), ", ")
        self.assertHTMLString(p(3), "Gaston et al. ")
        self.assertHTMLTag(p(4), "a", string="[2]", href="#gaston2015physics")

    def testNumberParentheticalHTML(self):
        text = "[!citep](slaughter2014framework, gaston2015physics)\n\n!bibtex bibliography"
        _, res = self.execute(text)
        # Parenthetical (!citep) citations render as just bracketed numbers.
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


class TestBibtexTitleMath(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, bibtex]
    TEXT = "[!cite](math_title)\n\n!bibtex bibliography"

    def setupContent(self):
        config = [
            dict(
                root_dir="python/MooseDocs/test/content",
                content=["extensions/bibtex_math_title.bib"],
            )
        ]
        return common.get_content(config, ".bib")

    def testTitleMathHTML(self):
        _, res = self.execute(self.TEXT)
        text = res.write()
        self.assertIn("Behavior of UO", text)
        self.assertIn("fuel at $5", text)
        self.assertIn('katex.render("_2"', text)
        self.assertIn("moose-katex-inline-equation", text)
        self.assertNotIn("$_2$", text)
        self.assertNotIn(r"\$5", text)


class TestBibtexToRIS(unittest.TestCase):
    def testRIS(self):
        entry = Entry(
            "article",
            persons={
                "author": [Person("Slaughter, Andrew E."), Person("Peterson, John W.")]
            },
            fields={
                "title": "Continuous integration for concurrent MOOSE framework and "
                "application development on GitHub",
                "journal": "Journal of Open Research Software",
                "year": "2015",
                "volume": "3",
                "number": "1",
            },
        )
        ris = bibtex.bibtex_to_ris(entry).splitlines()
        self.assertEqual(ris[0], "TY  - JOUR")
        self.assertIn("AU  - Slaughter, Andrew E.", ris)
        self.assertIn(
            "TI  - Continuous integration for concurrent MOOSE framework and "
            "application development on GitHub",
            ris,
        )
        self.assertIn("JO  - Journal of Open Research Software", ris)
        self.assertIn("PY  - 2015", ris)
        self.assertIn("VL  - 3", ris)
        self.assertEqual(ris[-1], "ER  - ")

    def testRISLatexAuthor(self):
        # Author names containing LaTeX must be converted to plain text.
        entry = Entry(
            "inproceedings",
            persons={"author": [Person("Andr\\v{s}, D.")]},
            fields={"title": "X", "year": "2014"},
        )
        ris = bibtex.bibtex_to_ris(entry)
        self.assertIn("AU  - Andr\u0161, D.", ris)
        self.assertNotIn("\\v", ris)

    def testRISNameParticles(self):
        # Name particles (e.g. "van") and suffixes (e.g. "Jr.") must be retained.
        entry = Entry(
            "article",
            persons={
                "author": [Person("van Genuchten, M. Th."), Person("Smith, Jr., John")]
            },
            fields={"title": "A closed-form equation", "year": "1980"},
        )
        ris = bibtex.bibtex_to_ris(entry).splitlines()
        self.assertIn("AU  - van Genuchten, M. Th.", ris)
        self.assertIn("AU  - Smith, John, Jr.", ris)


class TestBibtexExport(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, bibtex]
    TEXT = "[!cite](slaughter2014framework)\n\n!bibtex bibliography"

    def setupContent(self):
        config = [dict(root_dir="framework/doc/content", content=["bib/moose.bib"])]
        return common.get_content(config, ".bib")

    def testModalFormats(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())

        # The references list shows a single [Export] modal trigger.
        trigger = moosetree.find(
            res, lambda n: "moose-bibtex-modal" in (n.get("class") or "")
        )
        self.assertIsNotNone(trigger)
        self.assertEqual(trigger(0)["content"], "[Export]")

        # The modal offers BibTeX, RIS, and Plain Text exports.
        content = moosetree.find(
            res, lambda n: n.name == "div" and "modal-content" in (n.get("class") or "")
        )
        headings = [c(0)["content"] for c in content if c.name == "h6"]
        self.assertEqual(headings, ["BibTeX", "RIS", "Plain Text"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
