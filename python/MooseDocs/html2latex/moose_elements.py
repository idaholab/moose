
import os
import re
import logging
log = logging.getLogger(__name__)

import elements

def get_language(tag):
    """
    Helper function for extracting code highlight language from tag attributes.
    """
    # Map from lower case hljs name to latex listings name
    languages = {'c++':'C++', 'python':'Python'}

    lang = ''
    if 'class' in tag.attrs and 'hljs' in tag.attrs['class']:
        idx = tag.attrs['class'].index('hljs') + 1
        lang = tag.attrs['class'][idx]
        if lang.lower() in languages:
            lang = languages[lang.lower()]
        else:
            lang = ''
    return lang


def escape(content):
    """
    Escape special latex characters.
    """
    map = dict()
    map['_'] = '\\_'
    map['{'] = '\\{'
    map['}'] = '\\}'
    map['$'] = '\\$'
    map['&'] = '\\&'
    map['%'] = '\\%'
    map['\\'] = '\\textbackslash '
    map['~'] = '\\textasciitilde '
    map['^'] = '\\textasciicircum '
    def sub(match):
        return map[match.group(1)]
    return re.sub(r'([_{}$\\%&~^])', sub, content)


def admonition_preamble():
    """
    Returns commands to create admonition in latex.
    """
    out = ['\\usepackage{xparse}']
    out += ['\\usepackage{tabularx}']
    out += ['\\usepackage[table]{xcolor}']
    out += ['\\definecolor{code-background}{HTML}{ECF0F1}']
    out += ['\\definecolor{info-title}{HTML}{528452} \\definecolor{info}{HTML}{82E0AA}']
    out += ['\\definecolor{note-title}{HTML}{3A7296} \\definecolor{note}{HTML}{85C1E9}']
    out += ['\\definecolor{important-title}{HTML}{B100B0} \\definecolor{important}{HTML}{FF00FF}']
    out += ['\\definecolor{warning-title}{HTML}{968B2B} \\definecolor{warning}{HTML}{FFEC46}']
    out += ['\\definecolor{danger-title}{HTML}{B14D00} \\definecolor{danger}{HTML}{F75E1D}']
    out += ['\\definecolor{error-title}{HTML}{940000} \\definecolor{error}{HTML}{FFB4B4}']
    cmd = '\\DeclareDocumentCommand{\\admonition}{O{warning-title}O{warning}mm}\n'
    cmd += '{\n'
    cmd += '  \\rowcolors{1}{#1}{#2}\n'
    cmd += '  \\renewcommand{\\arraystretch}{1.5}\n'
    cmd += '  \\begin{tabularx}{\\textwidth}{X}\n'
    cmd += '  \\textcolor[rgb]{1,1,1}{\\textbf{#3}} \\\\ #4\n'
    cmd += '  \\end{tabularx}\n'
    cmd += '  \\rowcolors{1}{white}{white}\n'
    cmd += '}\n'
    return out + [cmd]


def listings_settings():
    out = ['\\usepackage[table]{xcolor}']
    out += ['\\definecolor{code-background}{HTML}{ECF0F1}']
    out += ['\\lstset{basicstyle=\\footnotesize\\rmfamily, breaklines=true, backgroundcolor=\\color{code-background}}']
    return out


class moose_table(elements.BlockElement):
    """
    Builds table fitted to the width of the document.
    """
    name = 'table'
    def convert(self, tag, content):
        tr = tag.tr
        td = tr.find_all('th')
        frmt = ['l']*len(td)
        frmt[-1] = 'X'
        return '\\begin{tabularx}{\linewidth}{%s}%s\\end{tabularx}' % (''.join(frmt), content)
    def preamble(self):
        return ['\\usepackage{tabularx}']


class admonition_div(elements.BlockElement):
    """
    Create an admonition in latex, assumes that the \admonition command is defined
    in the latex preamble.
    """
    name = 'div'
    attrs = ['class']

    def test(self, tag):
        return super(admonition_div, self).test(tag) and 'admonition' in tag.attrs['class']

    def convert(self, tag, content):
        atype = tag.attrs['class'][tag.attrs['class'].index('admonition')+1]
        title = self.content(tag.contents[1])
        #Message is optional
        if len(tag.contents) < 4:
            msg = ''
        else:
            msg = self.content(tag.contents[3])
        return '\\admonition[%s-title][%s]{%s}{%s}' % (atype, atype, title, msg)

    def preamble(self):
        return admonition_preamble()


class moose_hide_hr(elements.hr):
    """
    Hides horizontal hr tags in latex.
    """
    def convert(self, tag, content):
        return ''


class moose_inline_code(elements.InlineElement):
    """
    Improved inline code that wraps lines and escapes latex special commands.
    """
    name = 'code'
    def convert(self, tag, content):
        return '\\textrm{%s}' % escape(content)


class moose_pre(elements.pre):
    """
    Uses listing package rather than verbatim for code.
    """
    def convert(self, tag, content):
        lang = get_language(tag.contents[0])
        return '\\begin{lstlisting}[language=%s]\n%s\\end{lstlisting}' % (lang, content)
    def preamble(self):
        return ['\\usepackage{listings}'] + listings_settings()


class moose_pre_code(elements.pre_code):
    """
    Handles the code environments that have the filename as a heading.
    """
    def convert(self, tag, content):
        lang = get_language(tag.contents[0])
        return '\\begin{lstlisting}[language=%s]\n%s\\end{lstlisting}' % (lang, self.content(tag.contents[0]))
    def preamble(self):
        return ['\\usepackage{listings}'] + listings_settings()


class moose_code_div(elements.BlockElement):
    """
    Create a listing block for code blocks generated by MooseMarkdownExtension.
    """
    name = 'div'
    attrs = ['class']

    def __init__(self):
        super(moose_code_div, self).__init__()

    def test(self, tag):
        return super(moose_code_div, self).test(tag) and 'moosedocs-code-div' in tag.attrs['class']

    def convert(self, tag, content):

        # Locate the code
        for code in tag.descendants:
            if code.name == 'code':
                break

        # Determine the language and include the listings conversion from html to listing names
        lang = get_language(code)
        return '\\begin{lstlisting}[caption=%s, language=%s]\n%s\\end{lstlisting}' % (tag.contents[0], lang, self.content(code))

    def preamble(self):
        out = '\\usepackage{caption}\n'
        out += '\\DeclareCaptionFormat{listing}{#1#2#3}\n'
        out += '\\captionsetup[lstlisting]{format=listing, singlelinecheck=false, margin=0pt, font={sf}}'
        return ['\\usepackage{listings}', out]


class moose_internal_links(elements.a):
    """
    Create section-based hyper links
    """
    def test(self, tag):
        return super(moose_internal_links, self).test(tag) and tag['href'].startswith('#')

    def convert(self, tag, content):
        return '\\hyperref[sec:%s]{%s}' % (tag['href'][1:], content)

    def preamble(self):
        return ['\\usepackage{hyperref}']


class moose_markdown_links(elements.a):
    """
    <a> tag that links to website if markdown file is provided.
    """
    def __init__(self, site=None):
        self._site = site
        self._path = None # when testing the path is determined and used in

    def test(self, tag):
        """
        Test if the <a> tag contains a .md file include handling # for page section links.
        """
        if super(moose_markdown_links, self).test(tag):
            if tag['href'].endswith('.md'):
                self._path = tag['href'][:-3].strip('/')
                return True
            elif '.md#' in tag['href']:
                self._path = tag['href'].replace('.md', '/')
                return True

        self._path = None
        return False

    def convert(self, tag, content):
        url = '{}/{}'.format(self._site, self._path)
        return '\\href{%s}{%s}' % (url, content)

    def preamble(self):
        return ['\\usepackage{hyperref}']


class moose_img(elements.img):
    """
    Handles images with MOOSE markdown by doing some extra work to make sure path is correct.
    """

    def convert(self, tag, content):

        path = tag.attrs['src']
        if not os.path.exists(path):
            lpath = path.strip('/')
            if os.path.exists(lpath):
                path = lpath

        if not os.path.exists(path):
            log.error('Image file does not exist: {}'.format(path))

        path = os.path.abspath(path)
        width = tag.attrs.get('width', '\linewidth')
        return "\\begin{center}\n\\includegraphics[width=%s]{%s}\n\\end{center}" % (width, path)

    def preamble(self):
        return ['\\usepackage{graphicx}']

class moose_bib(elements.ol):
    """
    Convert html bibliography (from MooseBibtex) to the correct latex entry.
    """
    attrs = ['data-moose-bibfiles']
    def convert(self, tag, content):
        bibfiles = eval(tag.attrs['data-moose-bibfiles'])
        return '\\bibliographystyle{unsrtnat}\n\\bibliography{%s}' % ','.join(bibfiles)
    def preamble(self):
        return ['\\usepackage{natbib}']

class moose_bib_span(elements.span):
    """
    Convert the cite command from MooseBibtex to the proper latex cite command.
    """
    attrs = ['data-moose-cite']
    def convert(self, tag, content):
        return tag.attrs['data-moose-cite']

class moose_slider(elements.BlockElement):
    """
    Produces error for unsupported syntax for PDF creation.
    """
    name = 'div'
    attrs = ['class']
    def test(self, tag):
        return super(moose_slider, self).test(tag) and 'slider' in tag.attrs['class']

    def convert(self, tag, content):
        log.warning("!slideshow markdown is not currently supported for latex/pdf output.")
        return '\\admonition[error-title][error]{ERROR: Un-supported Markdown!}{MOOSE Slider is not currently supported for pdf output.}'
    def preamble(self):
        return admonition_preamble()

class moose_buildstatus(elements.BlockElement):
    """
    Produces error for unsupported syntax for PDF creation.
    """
    name = 'div'
    attrs = ['class']
    def test(self, tag):
        return super(moose_buildstatus, self).test(tag) and 'moose-buildstatus' in tag.attrs['class']

    def convert(self, tag, content):
        log.warning("!buildstatus markdown is not currently supported for latex/pdf output.")
        return '\\admonition[error-title][error]{ERROR: Un-supported Markdown!}{MOOSE build status (!buildstatus) is not currently supported for pdf output.}'
    def preamble(self):
        return admonition_preamble()


class moose_diagram(elements.BlockElement):
    """
    Produce and error for unsupported syntax for diagrams.

    @TODO: graphviz needs to support png output, the one in the MOOSE package does not.
    """
    name = 'img'
    attrs = ['class']
    def test(self, tag):
        return super(moose_diagram, self).test(tag) and 'moose-diagram' in tag.attrs['class']

    def convert(self, tag, content):
        log.warning("Dot diagram markdown syntax is not currently supported for latex/pdf output.")
        return '\\admonition[error-title][error]{ERROR: Un-supported Markdown!}{Dot diagram markdown syntax is not currently supported for latex/pdf output.}'
    def preamble(self):
        return admonition_preamble()
