import bs4

class Element(object):
    """
    Base class for converting html tag to latex.
    """
    name = None
    attrs = []

    def __init__(self):
        pass

    def test(self, tag):
        if tag.name == self.name and all([a in tag.attrs for a in self.attrs]):
            return True
        return False

    @staticmethod
    def content(tag, escape=True):
        """
        Creates complete content contained within this tag and escapes _ in raw text.
        """
        return u''.join([unicode(c) for c in tag.children])

    def convert(self, tag, content):
        return None

    def prefix(self, tag, level=0):
        pass

    def preamble(self):
        return []

class BlockElement(Element):
    """
    BlockElements begin with a new line.
    """
    def prefix(self, tag):
        return '\n'

class InlineElement(Element):
    """
    InlineElements are 'inline', says Cpt. Obvious.
    """
    def prefix(self, tag, level=0):
        return ''

class h1(BlockElement):
    name = 'h1'
    attrs = ['id']
    def __init__(self, command='section'):
        self.command = command
    def convert(self, tag, content):
        return '\\%s{%s\\label{sec:%s}}' % (self.command, content, tag["id"])

class h2(BlockElement):
    name = 'h2'
    attrs = ['id']
    def __init__(self, command='subsection'):
        self.command = command
    def convert(self, tag, content):
        return '\\%s{%s\\label{sec:%s}}' % (self.command, content, tag["id"])

class h3(BlockElement):
    name = 'h3'
    attrs = ['id']
    def __init__(self, command='subsubsection'):
        self.command = command
    def convert(self, tag, content):
        return '\\%s{%s\\label{sec:%s}}' % (self.command, content, tag["id"])

class h4(BlockElement):
    name = 'h4'
    attrs = ['id']
    def __init__(self, command='textbf'):
        self.command = command
    def convert(self, tag, content):
        return '\\%s{%s\\label{sec:%s}}' % (self.command, content, tag["id"])

class h5(BlockElement):
    name = 'h5'
    attrs = ['id']
    def __init__(self, command='underline'):
        self.command = command
    def convert(self, tag, content):
        return '\\%s{%s\\label{sec:%s}}' % (self.command, content, tag["id"])

class h6(BlockElement):
    name = 'h5'
    attrs = ['id']
    def __init__(self, command='emph'):
        self.command = command
    def convert(self, tag, content):
        return '\\%s{%s\\label{sec:%s}}' % (self.command, content, tag["id"])

class div(BlockElement):
    name = 'div'
    def convert(self, tag, content):
        return content

class pre(BlockElement):
    name = 'pre'
    def convert(self, tag, content):
        return '\\begin{verbatim}\n%s\\end{verbatim}' % content

class pre_code(BlockElement):
    name = 'pre'
    def test(self, tag):
        return super(pre_code, self).test(tag) and (tag.contents[0].name == 'code')
    def convert(self, tag, content):
        return '\\begin{verbatim}\n%s\\end{verbatim}' % self.content(tag.contents[0])

class table(BlockElement):
    name = 'table'
    def convert(self, tag, content):
        tr = tag.find_all('tr')
        return '\\begin{tabular}{%s}%s\\end{tabular}' % ('l'*len(tr), content)

class thead(InlineElement):
    name = 'thead'
    def convert(self, tag, content):
        return '\\hline%s' % content

class tbody(InlineElement):
    name = 'tbody'
    def convert(self, tag, content):
        return '\\hline%s\\hline' % content

class tr(InlineElement):
    name = 'tr'
    def convert(self, tag, content):
        items = [Element.content(c) for c in tag.find_all('td')]
        items += [Element.content(c) for c in tag.find_all('th')]
        return ' & '.join(items) + '\\\\'

class ol(BlockElement):
    name = 'ol'
    def convert(self, tag, content):
        return '\\begin{enumerate}%s\\end{enumerate}' % content

class ul(BlockElement):
    name = 'ul'
    def convert(self, tag, content):
        return '\\begin{itemize}%s\\end{itemize}' % content

class inline_equation(BlockElement):
    name = 'script'
    attrs = ['type']

    def test(self, tag):
        tf = super(inline_equation, self).test(tag)
        return tf and (tag['type'] == u'math/tex')
    def convert(self, tag, content):
        return '$%s$' % content

class equation(BlockElement):
    name = 'script'
    attrs = ['type']

    def test(self, tag):
        tf = super(equation, self).test(tag)
        return tf and (tag['type'] == u'math/tex; mode=display')

    def convert(self, tag, content):
        return content

class hr(BlockElement):
    name = 'hr'
    def convert(self, tag, content):
        return '\\hrule'

class figure(BlockElement):
    name = 'figure'
    def convert(self, tag, content):
        return "\\begin{figure}%s\\end{figure}" % content

class figcaption(InlineElement):
    name = 'figcaption'
    def convert(self, tag, content):
        return "\\caption{%s}" % content

class img(BlockElement):
    name = 'img'
    def convert(self, tag, content):
        return '\\includegraphics{%s}' % tag.attrs['src']
    def preamble(self):
        return ['\\usepackage{graphicx}']

class a(InlineElement):
    name = 'a'
    attrs = ['href']
    def convert(self, tag, content):
        return '\\href{%s}{%s}' % (tag['href'], content)
    def preamble(self):
        return ['\\usepackage{hyperref}']

class span(InlineElement):
    name = 'span'
    def convert(self, tag, content):
        return content

class p(InlineElement):
    name = 'p'
    def convert(self, tag, content):
        return '\\par\n%s' % content

class li(InlineElement):
    name = 'li'
    def convert(self, tag, content):
        return '\\item %s' % content

class code(InlineElement):
    name = 'code'
    def convert(self, tag, content):
        return '\\texttt{%s}' % content

class em(InlineElement):
    name = 'em'
    def convert(self, tag, content):
        return '\\emph{%s}' % content

class unknown(BlockElement):
    def convert(self, tag, content):
        return '\\begin{verbatim}\n%s\\end{verbatim}' % content
