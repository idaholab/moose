import re
from markdown.postprocessors import Postprocessor
import bs4

class MooseSlideContents(Postprocessor):
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
