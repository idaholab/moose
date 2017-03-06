from markdown.treeprocessors import Treeprocessor
from markdown.util import etree

class MooseContentScroll(Treeprocessor):
    """
    Adds a 'div' tag around h2 levels with class of 'section scrollspy' to allow scrollable contents
    on right-hand side of pages.
    """

    def __init__(self, markdown_instance=None, **kwargs):
        super(MooseContentScroll, self).__init__(markdown_instance)

    def run(self, root):
        """
        Adds section for materialize scrollspy
        """

        div = root.find('div')
        if (div is not None) and (div.get('id', None) == 'moose-markdown-content'):
            current = div
            parent = div
            for el in div.iter():
                if el.name == 'h2':
                    parent = current
                    current = etree.Element('div', id=tag.get('id', '#'))
                    current.set('class', "section scrollspy")

                if current != parent:
                    parent.append(current)
                    current.append(el)
                    parent.remove(el)
