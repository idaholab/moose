#!/usr/bin/env python
#pylint: disable=missing-docstring, anomalous-backslash-in-string
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
import os
import unittest

from MooseDocs import testing

class TestLatexMooseElements(testing.TestLatexBase):
    def test_moose_table(self):
        html = u'<thead><tr><th>h1</th><th>h2</th><th>h3</th></tr></thead>'
        html += u'<tbody><tr><td>1</td><td>2</td><td>3</td></tr><tr><td>2</td><td>4</td>' \
                 '<td>26</td></tr></tbody>'
        html += u'<tfoot><tr><td>f1</td><td>f2</td><td>f3</td></tr></tfoot>'
        gold = '\\hline\nh1 & h2 & h3 \\\\\n\\hline'
        gold += '1 & 2 & 3 \\\\\n2 & 4 & 26 \\\\\n'
        gold += '\\hline\nf1 & f2 & f3 \\\\\n\\hline'
        self.assertLaTeX(html, gold)

        html = '<table>' + html + '</table>'
        gold = '\\begin{tabularx}{\linewidth}{llX}\n' + gold + '\n\\end{tabularx}'
        self.assertLaTeX(html, gold)

    def test_admonition(self):
        html = u'<div class="admonition info">'
        html += u'<p class="admonition-title">The Title</p>'
        html += u'<p>Testing Testing.</p>'
        html += u'</div>'
        gold = '\\admonition[info-title][info]{The Title}{Testing Testing.}\n'
        self.assertLaTeX(html, gold)

    def test_admonition_no_title(self):
        html = u'<div class="admonition danger">'
        html += u'<p>Testing Testing.</p>'
        html += u'</div>'
        gold = '\\admonition[danger-title][danger]{}{Testing Testing.}\n'
        self.assertLaTeX(html, gold)

    def test_moose_img_caption(self):
        html = u'<p align="justify" class="moose-image-caption">Caption with ' \
                '<a href="http://foo.org">reference</a> inline.</p>'
        gold = '\\caption{Caption with \\href{http://foo.org}{reference} inline.}\n'
        self.assertLaTeX(html, gold)

    @unittest.skip('Needs Updating')
    def test_moose_img(self):

        html_file = os.path.join(self.WORKING_DIR, 'moose_img.html')
        html = self.soup(html_file)
        img = os.path.join('docs', 'media', 'rd100.png')

        # Figure, ID, and caption
        gold = u'\\begin{figure}\n'
        gold += u'\\label{thelabel}\n'
        gold += u'\\includegraphics[width=\linewidth]{%s}\n' % img
        gold += u'\\caption{The image caption with \href{http://foo.org}{reference} inline.}\n'
        gold += u'\\end{figure}'
        self.assertLaTeX(str(html), gold)

        # No caption
        html.find_all('div')[4].extract()
        gold = u'\\begin{figure}\n'
        gold += u'\\label{thelabel}\n'
        gold += u'\\includegraphics[width=\\linewidth]{%s}\n' % img
        gold += u'\\end{figure}'
        self.assertLaTeX(str(html), gold)

        # No label or caption
        del html.div.attrs['id']
        gold = u'\\begin{figure*}\n'
        gold += u'\\includegraphics[width=\\linewidth]{%s}\n' % img
        gold += u'\\end{figure*}'
        self.assertLaTeX(str(html), gold)

    def test_moose_hr(self):
        self.assertEqual(self.convert(u'<hr>'), u'')

    def test_button(self):
        self.assertEqual(self.convert(u'<button>copy</button>'), u'')

    def test_moose_pre_code(self):
        html = u'<pre class=" language-text"><button class="moose-copy-button btn"' \
                'data-clipboard-target="#moose-code-block-1">copy</button><code class="text ' \
                'language-text" id="moose-code-block-1">  params.addParam<int>("foo", 1, "doc");' \
                '</code></pre>'
        gold = u'\\begin{lstlisting}\n'
        gold += u'  params.addParam<int>("foo", 1, "doc");\n'
        gold += u'\\end{lstlisting}'
        self.assertLaTeX(html, gold)

        html = html.replace('language-text', 'language-python')
        gold = gold[:18] + '[language=Python]' + gold[18:]
        self.assertLaTeX(html, gold)

    def test_moose_code_div(self):
        html = u'<div class="moosedocs-code-div" style="max-height:200px;overflow-y:scroll;">' \
                '<a href="https://Foo.C">Foo.C</a>' \
                '<pre class=" language-text"><button class="moose-copy-button btn" ' \
                'data-clipboard-target="#moose-code-block-0">copy</button>' \
                '<code class="text language-text" ' \
                'id="moose-code-block-0">int x = 0;</code></pre></div>'
        gold = '\\begin{lstlisting}[caption=\\href{https://Foo.C}{Foo.C}]\n' \
                'int x = 0;\n' \
                '\\end{lstlisting}'

        html = html.replace('language-text', 'language-python')
        gold = gold[:19] + 'language=Python, ' + gold[19:]
        self.assertLaTeX(html, gold)

    def test_moose_cite(self):
        html = u'<span data-moose-cite="\cite{slaughter2015continuous}">' \
                '<a href="#slaughter2015continuous">Slaughter et al. (2015)</a></span>'
        gold = '\\cite{slaughter2015continuous}'
        self.assertLaTeX(html, gold)


class TestLatexMooseElementsDisableHRule(testing.TestLatexBase):
    """
    Test MooseExtension with hrule enabled (not default)
    """
    CONFIG = {'hrule':True}
    def test_moose_hr(self):
        """
        Converts <hr> --> \hrule
        """
        self.assertEqual(self.convert(u'<hr>'), '\n\\hrule\n')

if __name__ == '__main__':
    unittest.main(verbosity=2)
