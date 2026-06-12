# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
import sys
import uuid
import logging

from pybtex.plugin import find_plugin, PluginNotFound
from pybtex.database import BibliographyData, parse_file
from pybtex.database.input.bibtex import UndefinedMacro, Person
from pybtex.errors import set_strict_mode
from pylatexenc.latex2text import LatexNodes2Text

import moosetree

from ..common import exceptions
from ..base import components, LatexRenderer, MarkdownReader
from ..tree import tokens, html, latex
from . import core, command

LOG = logging.getLogger("MooseDocs.extensions.bibtex")


def make_extension(**kwargs):
    return BibtexExtension(**kwargs)


BibtexCite = tokens.newToken("BibtexCite", keys=[])
BibtexBibliography = tokens.newToken("BibtexBibliography", bib_style="")
BibtexList = tokens.newToken("BibtexList", BibtexBibliography, bib_files=None)

# Maps BibTeX entry types to RIS reference types; unknown types fall back to GEN.
RIS_TYPES = {
    "article": "JOUR",
    "book": "BOOK",
    "booklet": "BOOK",
    "inbook": "CHAP",
    "incollection": "CHAP",
    "inproceedings": "CPAPER",
    "conference": "CPAPER",
    "manual": "GEN",
    "mastersthesis": "THES",
    "phdthesis": "THES",
    "proceedings": "CONF",
    "techreport": "RPRT",
    "unpublished": "UNPB",
    "misc": "GEN",
}


def bibtex_to_ris(entry):
    """Convert a pybtex bibliography entry into an RIS-formatted string.

    RIS is the interchange format imported by reference managers used outside of
    LaTeX, such as Microsoft Word, EndNote, Zotero, and Mendeley. pybtex provides
    no RIS writer, so the common fields are mapped here by hand.
    """
    to_text = LatexNodes2Text().latex_to_text
    fields = entry.fields
    lines = [("TY", RIS_TYPES.get(entry.type, "GEN"))]

    persons = entry.persons.get("author") or entry.persons.get("editor") or []
    for person in persons:
        # RIS author names are "family, given, suffix"; the family name keeps any
        # particle (e.g. "van") and the suffix holds the lineage (e.g. "Jr.").
        last = to_text(" ".join(person.prelast_names + person.last_names))
        given = to_text(" ".join(person.first_names + person.middle_names))
        name = "{}, {}".format(last, given) if given else last
        if person.lineage_names:
            name += ", {}".format(to_text(" ".join(person.lineage_names)))
        lines.append(("AU", name))

    field_map = [
        ("TI", "title"),
        ("JO", "journal"),
        ("T2", "booktitle"),
        ("PY", "year"),
        ("VL", "volume"),
        ("IS", "number"),
        ("PB", "publisher"),
        ("CY", "address"),
        ("DO", "doi"),
        ("UR", "url"),
    ]
    for tag, field in field_map:
        if field in fields:
            lines.append((tag, to_text(fields[field])))

    if "pages" in fields:
        parts = fields["pages"].replace("--", "-").split("-")
        lines.append(("SP", parts[0].strip()))
        if len(parts) > 1:
            lines.append(("EP", parts[-1].strip()))

    lines.append(("ER", ""))
    return "\n".join("{}  - {}".format(tag, value) for tag, value in lines)


class BibtexExtension(command.CommandExtension):
    """
    Extension for BibTeX citations and bibliography.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config["duplicate_warning"] = (
            True,
            "Show a warning when duplicate entries detected.",
        )
        config["duplicates"] = (list(), "A list of duplicates that are allowed.")
        config["citation_style"] = (
            "author-year",
            "The inline citation style: 'author-year' (e.g. 'Smith et al. (2024)') "
            "or 'number' (e.g. '[1]').",
        )
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        self.__database = None
        self.__bib_files = list()
        self.__bib_file_database = dict()

    def preExecute(self):
        set_strict_mode(False)  # allow incorrectly formatted author/editor names

        # If this is invoked during a live serve, we need to recompile the list of '.bib' files and
        # read them again, otherwise there's no way to distinguish existing entries from duplicates
        self.__bib_files = []
        for node in self.translator.findPages(lambda p: p.source.endswith(".bib")):
            self.__bib_files.append(node.source)

        self.__database = BibliographyData()
        for bfile in self.__bib_files:
            try:
                db = parse_file(bfile)
                self.__bib_file_database[bfile] = db
            except UndefinedMacro as e:
                msg = "The BibTeX file %s has an undefined macro:\n%s"
                LOG.warning(msg, bfile, e)

            # TODO: https://bitbucket.org/pybtex-devs/pybtex/issues/93/
            #      databaseadd_entries-method-not-considering
            for key in db.entries:
                if key in self.__database.entries:
                    if self.get("duplicate_warning") and (
                        key not in self.get("duplicates")
                    ):
                        msg = "The BibTeX entry '%s' defined in %s already exists."
                        LOG.warning(msg, key, bfile)
                else:
                    self.__database.add_entry(key, db.entries[key])

    def preRead(self, page):
        """Initialize the page citations list."""
        page["citations"] = list()
        page["citation_numbers"] = None

    def citationNumbers(self, page):
        """Return a {key: number} map for a page, numbered in order of first
        citation appearance (deduplicated). Used by the 'number' citation style."""
        numbers = page.get("citation_numbers")
        if numbers is None:
            numbers = dict()
            for key in page.get("citations", list()):
                if key not in numbers:
                    numbers[key] = len(numbers) + 1
            page["citation_numbers"] = numbers
        return numbers

    def postTokenize(self, page, ast):
        if page["citations"]:
            has_bib = False
            for node in moosetree.iterate(ast):
                if node.name == "BibtexBibliography":
                    has_bib = True
                    break

            if not has_bib:
                core.Heading(ast, level=2, string="References")
                BibtexBibliography(ast, bib_style="plain")

    def database(self, bibfile=None):
        if bibfile is None:
            return self.__database
        else:
            return self.__bib_file_database[bibfile]

    def bibfiles(self):
        return self.__bib_files

    def extend(self, reader, renderer):
        self.requires(core, command)

        self.addCommand(reader, BibtexCommand())
        self.addCommand(reader, BibtexListCommand())
        self.addCommand(reader, BibtexReferenceComponent())

        renderer.add("BibtexCite", RenderBibtexCite())
        renderer.add("BibtexList", RenderBibtexList())
        renderer.add("BibtexBibliography", RenderBibtexBibliography())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage("natbib", "round")


class BibtexReferenceComponent(command.CommandComponent):
    COMMAND = ("cite", "citet", "citep", "nocite")
    SUBCOMMAND = None

    def createToken(self, parent, info, page, settings):
        keys = [key.strip() for key in info["inline"].split(",")]
        BibtexCite(parent, keys=keys, cite=info["command"])
        page["citations"].extend(keys)
        return parent


class BibtexCommand(command.CommandComponent):
    COMMAND = "bibtex"
    SUBCOMMAND = "bibliography"

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config["style"] = (
            "plain",
            "The BibTeX style (plain, unsrt, alpha, unsrtalpha).",
        )
        config["title"] = ("References", "The section title for the references.")
        config["title-level"] = (
            2,
            "The heading level for the section title for the references.",
        )
        return config

    def createToken(self, parent, token, page, settings):
        if settings["title"]:
            h = core.Heading(parent, level=int(settings["title-level"]))
            self.reader.tokenize(h, settings["title"], page, MarkdownReader.INLINE)
        BibtexBibliography(parent, bib_style=settings["style"])
        return parent


class BibtexListCommand(command.CommandComponent):
    COMMAND = "bibtex"
    SUBCOMMAND = "list"

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config["bib_files"] = (
            None,
            "The list of *.bib files to use for a complete citation list.",
        )
        return config

    def createToken(self, parent, token, page, settings):
        bfiles = settings["bib_files"]
        bib_files = list()
        if bfiles is None:
            bib_files = self.extension.bibfiles()
        else:
            for bfile in bfiles.split():
                for key in self.extension.bibfiles():
                    if key.endswith(bfile):
                        bib_files.append(key)

        BibtexList(parent, bib_files=bib_files)
        return parent


class RenderBibtexCite(components.RenderComponent):

    def createHTML(self, parent, token, page):

        cite = token["cite"]
        if cite == "nocite":
            return parent

        if self.extension.get("citation_style") == "number":
            return self._createNumberHTML(parent, token, page)

        citep = cite == "citep"
        if citep:
            html.String(parent, content="(")

        num_keys = len(token["keys"])
        for i, key in enumerate(token["keys"]):

            if key not in self.extension.database().entries:
                LOG.error("Unknown BibTeX key: %s", key)
                html.Tag(parent, "span", string=key, style="color:red;")
                continue

            entry = self.extension.database().entries[key]
            author = self._authorString(key, entry)

            form = "{}, {}" if citep else "{} ({})"
            year = entry.fields.get("year", None)
            if year is None:
                raise exceptions.MooseDocsException(
                    "Unable to locate year for bibtex entry '{}'", entry.key
                )
            html.Tag(
                parent, "a", href="#{}".format(key), string=form.format(author, year)
            )

            if citep:
                if num_keys > 1 and i != num_keys - 1:
                    html.String(parent, content="; ")
            else:
                if num_keys == 2 and i == 0:
                    html.String(parent, content=" and ")
                elif num_keys > 2 and i == num_keys - 2:
                    html.String(parent, content=", and ")
                elif num_keys > 2 and i != num_keys - 1:
                    html.String(parent, content=", ")

        if citep:
            html.String(parent, content=")")

        return parent

    def _authorString(self, key, entry):
        """Return the inline author label for an entry, e.g. 'Slaughter et al.'.

        Falls back to initials of the institution or organization when the entry
        has no author."""
        author_found = True
        if (
            not "author" in entry.persons.keys()
            and not "Author" in entry.persons.keys()
        ):
            author_found = False
            entities = ["institution", "organization"]
            for entity in entities:
                if entity in entry.fields.keys():
                    author_found = True
                    name = ""
                    for word in entry.fields[entity]:
                        if word[0].isupper():
                            name += word[0]
                    entry.persons["author"] = [Person(name)]

        if not author_found:
            msg = "No author, institution, or organization for {}"
            raise exceptions.MooseDocsException(msg, key)

        a = entry.persons["author"]
        n = len(a)
        if n > 2:
            author = "{} et al.".format(" ".join(a[0].last_names))
        elif n == 2:
            a0 = " ".join(a[0].last_names)
            a1 = " ".join(a[1].last_names)
            author = "{} and {}".format(a0, a1)
        else:
            author = " ".join(a[0].last_names)

        return LatexNodes2Text().latex_to_text(author)

    def _createNumberHTML(self, parent, token, page):
        """Render citations using bracketed numbers that match the reference list.

        Parenthetical citations (!citep) render as just the number, e.g. '[1, 2]'.
        Textual citations (!cite, !citet) prepend the author so the citation reads
        as part of the sentence, e.g. 'Slaughter et al. [1]'."""
        numbers = self.extension.citationNumbers(page)
        textual = token["cite"] != "citep"
        num_keys = len(token["keys"])
        if not textual:
            html.String(parent, content="[")
        for i, key in enumerate(token["keys"]):
            if key not in self.extension.database().entries:
                LOG.error("Unknown BibTeX key: %s", key)
                html.Tag(parent, "span", string=key, style="color:red;")
            elif textual:
                entry = self.extension.database().entries[key]
                html.String(
                    parent, content="{} ".format(self._authorString(key, entry))
                )
                html.Tag(
                    parent,
                    "a",
                    href="#{}".format(key),
                    string="[{}]".format(numbers.get(key)),
                )
            else:
                html.Tag(
                    parent,
                    "a",
                    href="#{}".format(key),
                    string=str(numbers.get(key)),
                )
            if i != num_keys - 1:
                html.String(parent, content=", ")
        if not textual:
            html.String(parent, content="]")
        return parent

    def createMaterialize(self, parent, token, page):
        self.createHTML(parent, token, page)

    def createLatex(self, parent, token, page):
        latex.Command(
            parent, token["cite"], string=",".join(token["keys"]), escape=False
        )
        return parent


class RenderBibtexBibliography(components.RenderComponent):

    def getCitations(self, parent, token, page):
        return page.get("citations", list())

    def createHTML(self, parent, token, page):

        try:
            style = find_plugin("pybtex.style.formatting", token["bib_style"])
        except PluginNotFound:
            msg = 'Unknown bibliography style "{}".'
            raise exceptions.MooseDocsException(msg, token["bib_style"])

        citations = self.getCitations(parent, token, page)
        formatted_bibliography = style().format_bibliography(
            self.extension.database(), citations
        )

        if formatted_bibliography.entries:
            html_backend = find_plugin("pybtex.backends", "html")
            div = html.Tag(parent, "div", class_="moose-bibliography")
            ol = html.Tag(div, "ol")

            backend = html_backend(encoding="utf-8")
            entries = list(formatted_bibliography)
            if self.extension.get("citation_style") == "number":
                numbers = self.extension.citationNumbers(page)
                entries.sort(key=lambda e: numbers.get(e.key, len(numbers) + 1))
            for entry in entries:
                text = entry.text.render(backend)
                html.Tag(ol, "li", id_=entry.key, string=text)

            return ol

        else:
            html.String(parent, content="No citations exist within this document.")

    def createMaterialize(self, parent, token, page):
        ol = self.createHTML(parent, token, page)
        if ol is None:
            return

        for child in ol.children:
            key = child["id"]
            entry = self.extension.database().entries[key]

            db = BibliographyData()
            db.add_entry(key, entry)
            # The 'language-*' class lets Prism wrap each block in its toolbar
            # (adding the Copy button and overflow scrolling). RIS and plain text
            # have no Prism grammar, so 'language-none' gives them the same toolbar
            # without syntax coloring.
            formats = [
                ("BibTeX", db.to_string("bibtex"), "language-latex"),
                ("RIS", bibtex_to_ris(entry), "language-none"),
                (
                    "Plain Text",
                    self._plainText(key, token["bib_style"]),
                    "language-none",
                ),
            ]

            m_id = uuid.uuid4()
            html.Tag(
                child,
                "a",
                style="padding-left:10px;",
                class_="modal-trigger moose-bibtex-modal",
                href="#{}".format(m_id),
                string="[Export]",
            )

            modal = html.Tag(child, "div", class_="modal", id_=m_id)
            content = html.Tag(modal, "div", class_="modal-content")
            for name, text, lang in formats:
                html.Tag(content, "h6", string=name)
                pre = html.Tag(content, "pre", style="line-height:1.25;")
                html.Tag(pre, "code", class_=lang, string=text)

        return ol

    def _plainText(self, key, bib_style):
        """Render a single citation as a plain-text reference string."""
        style = find_plugin("pybtex.style.formatting", bib_style or "plain")
        formatted = style().format_bibliography(self.extension.database(), [key])
        backend = find_plugin("pybtex.backends", "plaintext")(encoding="utf-8")
        for entry in formatted:
            return entry.text.render(backend)
        return ""

    def createLatex(self, parent, token, page):
        pass


class RenderBibtexList(RenderBibtexBibliography):
    def getCitations(self, parent, token, page):
        citations = list()
        for bfile in token["bib_files"]:
            citations += self.extension.database(bfile).entries.keys()
        return citations
