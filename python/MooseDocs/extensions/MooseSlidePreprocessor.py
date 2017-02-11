import re
from markdown.preprocessors import Preprocessor
import MooseDocs

class MooseSlidePreprocessor(Preprocessor):
    """
    Creates sections breaks for presentations with Reveal.js.
    """

    def run(self, lines):
        """
        Searches the raw markdown lines for '---' and '--' and replaces them with the appropriate <section> tags.

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

        section = '\n\n{}\n\n{}\n\n{}\n\n'.format(self.markdown.htmlStash.store(start_section, safe=True), section, self.markdown.htmlStash.store(end_section, safe=True))
        return section

    def _getSlideID(self, section):
        """
        Helper for getting slide name
        """
        match = re.search(r'#+\s*(.*?)\s*\n', section)
        if match:
            return MooseDocs.html_id(match.group(1))
        return None
