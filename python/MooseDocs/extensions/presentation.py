#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring

import re
import logging

import bs4
import markdown
from markdown.preprocessors import Preprocessor
from markdown.postprocessors import Postprocessor

import MooseDocs

LOG = logging.getLogger(__name__)

class PresentationExtension(markdown.Extension):
    """
    Adds necessary compoments for creating presentation slides using Reveal.js.
    """
    def __init__(self, *args, **kwargs):
        self.config = dict()
        super(PresentationExtension, self).__init__(*args, **kwargs)

    def extendMarkdown(self, md, md_globals):
        """
        Builds the extensions for MOOSE Presentation Slides, which uses Reveal.js.
        """
        md.registerExtension(self)
        md.preprocessors.add('moose_slides', SlidePreprocessor(markdown_instance=md), '_end')
        md.postprocessors.add('moose_slide_contents',
                              SlideContentsPostprocessor(markdown_instance=md),
                              '<moose_template')


def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """
    Return the PresentationExtension object.
    """
    return PresentationExtension(*args, **kwargs)


class SlidePreprocessor(Preprocessor):
    """
    Creates sections breaks for presentations with Reveal.js.
    """

    def run(self, lines):
        """
        Searches the raw markdown lines for '---' and '--' and replaces them with the appropriate
        <section> tags.

        Args:
          lines[list]: List of markdown lines to preprocess.
        """

        # Break the lines at '---' items
        content = '\n'.join(lines)
        sections = content.split('---\n')

        # Loop through the sections
        for i, section in enumerate(sections):

            # Locate the parent name
            parent = self._getSlideID(section)

            # Look for sub-sections ('--') breaks
            subsections = section.split('--\n')
            has_subsections = len(subsections) > 1
            if has_subsections:
                for j, subsection in enumerate(subsections):
                    subsections[j] = self._injectSection(subsection, parent=parent)
                section = '\n'.join(subsections)

            # Inject '---' breaks
            sections[i] = self._injectSection(section, add_id=not has_subsections)

        return '\n'.join(sections).split('\n')

    def _injectSection(self, section, add_id=True, parent=None):
        """
        Helper for injecting html placeholders with the <section>, </section> tags.
        """

        # Build slide attributes
        match = re.search(r'(?<!`)<!--\s\.slide(.*?)\s*-->', section)
        attr = []
        if match:
            attr.append(match.group(1))
            section = section.replace(match.group(0), '', 1)

        # Slide id
        if add_id:
            htmlid = self._getSlideID(section)
            if htmlid:
                if parent and parent != htmlid:
                    htmlid = '{}-{}'.format(parent, htmlid)
                attr.append('id="{}"'.format(htmlid))

        # Define section tags
        start_section = u'<section {}>'.format(' '.join(attr))
        end_section = u'</section>'

        strt = self.markdown.htmlStash.store(start_section, safe=True)
        stop = self.markdown.htmlStash.store(end_section, safe=True)
        section = '\n\n{}\n\n{}\n\n{}\n\n'.format(strt, section, stop)
        return section

    @staticmethod
    def _getSlideID(section):
        """
        Helper for getting slide name
        """
        match = re.search(r'#+\s*(.*?)\s*\n', section)
        if match:
            return MooseDocs.html_id(match.group(1))
        return None


class SlideContentsPostprocessor(Postprocessor):
    """
    Adds ability to create table of contents for vertical slide groups.
    """
    RE = r'!subtoc'

    def run(self, text):
        """
        Inject the contents for vertical sections.
        """
        levels = ['h2']

        soup = bs4.BeautifulSoup(text, 'html.parser')

        for section in soup.find_all("section", recursive=False):
            headings = section.find_all(levels)
            for subsection in section.find_all("section", recursive=False):
                for item in subsection.find_all():
                    match = re.search(self.RE, unicode(item.string))
                    if match:
                        item.replace_with(self._contents(headings))
                    elif item in headings:
                        headings.remove(item)

        return soup.prettify()

    @staticmethod
    def _contents(headings):
        """
        Builds the contents given the supplied list of headings
        """
        soup = bs4.BeautifulSoup('', 'html.parser')
        ul = soup.new_tag('ul')
        for h in headings:
            li = soup.new_tag('li')
            a = soup.new_tag('a')
            a.string = h.string
            a['href'] = '#/{}'.format(h.parent['id'])
            li.append(a)
            ul.append(li)

        return ul
