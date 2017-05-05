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

import unittest

from MooseDocs import testing
from MooseDocs.html2latex import BasicExtension, Translator

class TestLatexElements(testing.TestLatexBase):
    EXTENSIONS = [BasicExtension]

    def test_a(self):
        html = u'<a href="foo">bar</a>'
        self.assertLaTeX(html, r'\href{foo}{bar}')

    def test_li(self):
        html = u'<li>cont_ent</li>'
        gold = r'\item cont\_ent'
        self.assertLaTeX(html, gold)

    def test_ul(self):
        html = u'<ul><li>one</li><li>two%</li></ul>'
        gold = '\\begin{itemize}\n\\item one\n\\item two\%\n\\end{itemize}'
        self.assertLaTeX(html, gold)

    def test_ol(self):
        html = u'<ol><li>one</li><li>two</li></ol>'
        gold = '\\begin{enumerate}\n\\item one\n\\item two\n\\end{enumerate}'
        self.assertLaTeX(html, gold)

    def test_headings(self):
        headings = ['section', 'subsection', 'subsubsection', 'textbf', 'underline', 'emph']
        text = ['One', 'Two', 'Three', 'Four', 'Five', 'Six']
        for i, h in enumerate(headings):
            html = u'<h{0}>Heading {1}</h{0}>'.format(i+1, text[i])
            gold = u'\%s{Heading %s}' % (h, text[i])
            self.assertLaTeX(html, gold)

            html = u'<h{0} id="heading-{2}">Heading {1}</h{0}>'.format(i+1, text[i],
                                                                       text[i].lower())
            gold = u'\%s{Heading %s\label{heading-%s}}' % (h, text[i], text[i].lower())
            self.assertLaTeX(html, gold)

    def test_div(self):
        html = u'<div>This is some content with <a href="foo">link</a> in the middle.</div>'
        gold = 'This is some content with \href{foo}{link} in the middle.'
        self.assertLaTeX(html, gold)

    def test_pre(self):
        html = u'<pre>double x = 2;\nx += 2;</pre>'
        gold = '\\begin{verbatim}\ndouble x = 2;\nx += 2;\n\\end{verbatim}'
        self.assertLaTeX(html, gold)

    def test_code(self):
        html = u'<code>double x = 2;</code>'
        gold = '\\texttt{double x = 2;}'
        self.assertLaTeX(html, gold)

        html = u'<code>$</code>'
        gold = '\\texttt{\$}'
        self.assertLaTeX(html, gold)

    def test_p(self):
        html = u'<p>This is a paragraph.</p>'
        gold = '\n\\par\nThis is a paragraph.'
        self.assertLaTeX(html, gold)

        html = u'<p>This is a paragraph.</p><p>And this is another.</p>'
        gold = '\n\\par\nThis is a paragraph.\n\\par\nAnd this is another.'
        self.assertLaTeX(html, gold)

    def test_nested_inline_code(self):
        html = u'<p>Inline math <code>$</code> works.</p>'
        gold = '\n\\par\nInline math \\texttt{\\$} works.'
        self.assertLaTeX(html, gold)

    def test_pre_code(self):
        html = u'<pre><code>double x = 2;\nx += 2;</code></pre>'
        gold = '\\begin{verbatim}\ndouble x = 2;\nx += 2;\n\\end{verbatim}'
        self.assertLaTeX(html, gold)

    def test_hr(self):
        html = u'<hr>'
        gold = '\n\\hrule\n'
        self.assertLaTeX(html, gold)

    def test_span(self):
        html = u'<span>Some text</span>'
        gold = 'Some text'
        self.assertLaTeX(html, gold)

    def test_em(self):
        html = u'<em>slanty</em>'
        gold = '\\emph{slanty}'
        self.assertLaTeX(html, gold)

    def test_inline_equation(self):
        html = u'<script type="math/tex">x+y</script>'
        gold = '$x+y$'
        self.assertLaTeX(html, gold)

    def test_block_equation(self):
        html = u'<script type="math/tex; mode=display">x+y</script>'
        gold = '\\begin{equation}\nx+y\n\\end{equation}'
        self.assertLaTeX(html, gold)

    def test_th_td(self):
        html = u'<th>1</th><th>2</th><th>3</th>'
        gold = '1 & 2 & 3 \\\\'
        self.assertLaTeX(html, gold)
        self.assertLaTeX(html.replace('th', 'td'), gold)

    def test_tr(self):
        html = u'<tr><th>1</th><th>2</th><th>3</th></tr><tr><th>2</th><th>4</th><th>6</th></tr>'
        gold = '1 & 2 & 3 \\\\\n2 & 4 & 6 \\\\\n'
        self.assertLaTeX(html, gold)

    def test_thead_tfoot(self):
        html = u'<thead><tr><th>1</th><th>2</th><th>3</th></tr></thead>'
        gold = '\\hline\n1 & 2 & 3 \\\\\n\\hline'
        self.assertLaTeX(html, gold)
        self.assertLaTeX(html.replace('thead', 'tfoot'), gold)

    def test_tbody(self):
        html = u'<tbody><tr><td>1</td><td>2</td><td>3</td></tr><tr><td>2</td><td>4</td><td>6</td>' \
                '</tr></tbody>'
        gold = '1 & 2 & 3 \\\\\n2 & 4 & 6 \\\\\n'
        self.assertLaTeX(html, gold)

    def test_thead_tbody_tfoot_table(self):
        html = u'<thead><tr><th>h1</th><th>h2</th><th>h3</th></tr></thead>'
        html += u'<tbody><tr><td>1</td><td>2</td><td>3</td></tr><tr><td>2</td><td>4</td><td>6' \
                 '</td></tr></tbody>'
        html += u'<tfoot><tr><td>f1</td><td>f2</td><td>f3</td></tr></tfoot>'
        gold = '\\hline\nh1 & h2 & h3 \\\\\n\\hline'
        gold += '1 & 2 & 3 \\\\\n2 & 4 & 6 \\\\\n'
        gold += '\\hline\nf1 & f2 & f3 \\\\\n\\hline'
        self.assertLaTeX(html, gold)

        html = '<table>' + html + '</table>'
        gold = '\\begin{tabular}{lll}\n' + gold + '\n\\end{tabular}'
        self.assertLaTeX(html, gold)

    def test_figure(self):
        html = u'<figure id="foo">content</figure>'
        gold = '\\begin{figure}\n\\label{foo}\ncontent\n\\end{figure}'
        self.assertLaTeX(html, gold)

        html = u'<figure>content</figure>'
        gold = '\\begin{figure*}\ncontent\n\\end{figure*}'
        self.assertLaTeX(html, gold)

    def test_figcaption(self):
        html = u'<figcaption>a caption</figcaption>'
        gold = '\\caption{a caption}'
        self.assertLaTeX(html, gold)

    def test_img(self):
        html = u'<img src="file.png">'
        gold = '\\includegraphics{file.png}\n'
        self.assertLaTeX(html, gold)

    def test_figure_figcaption_img(self):
        html = u'<figure><img src="file.png"><figcaption>a caption</figcaption></figure>'
        gold = '\\begin{figure*}\n\\includegraphics{file.png}\n\\caption{a caption}\n\\end{figure*}'
        self.assertLaTeX(html, gold)

    def test_center(self):
        html = u'<center>this</center>'
        gold = '\\begin{center}\nthis\n\\end{center}'
        self.assertLaTeX(html, gold)

    def test_escape(self):
        for key, value in Translator.ESCAPE_MAP.iteritems():
            html = u'<p>{}</p>'.format(key)
            gold = "\n\par\n{}".format(value)
            self.assertLaTeX(html, gold)

    def test_unknown(self):
        html = u'<unknown>something</unknown>'
        gold = '\\begin{verbatim}\n<unknown>something</unknown>\n\\end{verbatim}'
        self.assertLaTeX(html, gold)

    def test_latex_data(self):
        html = u'<div data-latex-content="foo"><p>Some text that will get removed.</p></div>'
        self.assertLaTeX(html, 'foo')

if __name__ == '__main__':
    unittest.main(verbosity=2)
