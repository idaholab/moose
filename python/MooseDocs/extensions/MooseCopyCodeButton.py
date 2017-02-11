from markdown.treeprocessors import Treeprocessor
from markdown.util import etree


class MooseCopyCodeButton(Treeprocessor):
    """
    Adds "copy" button to code blocks and corrects the language class for working with Prism.js
    """

    def __init__(self, markdown_instance=None, **kwargs):
        super(MooseCopyCodeButton, self).__init__(markdown_instance)

    def run(self, root):
        """
        Search the tree for <pre><code> blocks and add copy button.
        """
        count = 0
        for pre in root.iter('pre'):
            code = pre.find('code')
            if code is not None:
                pre.set('class', 'language-{}'.format(code.get('class', 'text')))

                id = code.get('id', 'moose-code-block-{}'.format(count))
                code.set('id', id)

                btn = etree.Element('button')
                btn.set('class', 'moose-copy-button btn')
                btn.set('data-clipboard-target', '#{}'.format(id))
                btn.text = 'copy'

                pre.insert(0, btn)
                count += 1
