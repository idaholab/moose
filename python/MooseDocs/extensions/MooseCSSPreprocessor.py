import re
from markdown.preprocessors import Preprocessor
from markdown.util import etree
from MooseCommonExtension import MooseCommonExtension


class MooseCSSPreprocessor(Preprocessor, MooseCommonExtension):
    """
    Allows the !css syntax to work with lists and tables.
    """

    START_CHARS = ['1', '*', '-', '|']

    def __init__(self, markdown_instance=None, **kwargs):
        MooseCommonExtension.__init__(self, **kwargs)
        Preprocessor.__init__(self, markdown_instance)

    def run(self, lines):
        """
        Searches the raw markdown lines for '!css followed by a line beginning with '*', '-', or '1' and creates
        a div around the area if found.

        Args:
          lines[list]: List of markdown lines to preprocess.
        """

        # Break the lines at '---' items
        content = '\n'.join(lines)
        content = re.sub(r'^(!css\s*(.*?)\n)(.*?)^$', self._injectListCSS, content, flags=re.MULTILINE|re.DOTALL)
        return content.split('\n')

    def _injectListCSS(self, match):
        """
        Substitution function.
        """
        if match.group(3).strip()[0] in self.START_CHARS:
            settings = self.getSettings(match.group(2))
            string = ['{}={}'.format(key, str(value)) for key, value in settings.iteritems() if value]
            strt = self.markdown.htmlStash.store(u'<div {}>'.format(' '.join(string)), safe=True)
            stop = self.markdown.htmlStash.store(u'</div>', safe=True)
            return '\n\n{}\n\n{}\n\n{}\n\n'.format(strt, match.group(3), stop)
