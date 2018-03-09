"""Defines Renderer objects that convert AST (from Reader) into an output format."""
import os
import re
import logging
import traceback
import uuid

import anytree

import mooseutils

import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions, mixins
from MooseDocs.tree import html, latex, base, page

LOG = logging.getLogger(__name__)

class Renderer(mixins.ConfigObject, mixins.TranslatorObject, mixins.ComponentObject):
    """
    Base renderer for converting AST to an output format.
    """

    #:[str] The name of the method to call on RendererComponent objects.
    METHOD = None

    def __init__(self, **kwargs):
        mixins.ConfigObject.__init__(self, **kwargs)
        mixins.TranslatorObject.__init__(self)
        mixins.ComponentObject.__init__(self)
        self.__functions = dict()  # functions on the RenderComponent to call

    def add(self, token, component):
        """
        Associate a RenderComponent object with a token type.

        Inputs:
            token[type]: The token type (not instance) to associate with the supplied component.
            compoment[RenderComponent]: The component to execute with the associated token type.
        """
        common.check_type("token", token, type)
        common.check_type("component", component, MooseDocs.base.components.RenderComponent)
        if self.initialized(): # allow use without Translator object
            component.init(self.translator)
        self.addComponent(component)
        self.__functions[token] = self._method(component)

    def reinit(self):
        """
        Call reinit() method of the RenderComponent objects.
        """
        for comp in self.components:
            comp.reinit()

    def render(self, ast): #pylint: disable=unused-argument
        """
        Render the supplied AST (abstract).

        This method is designed to be overridden to create the desired output tree, see the
        HTMLRenderer and/or LatexRenderer for examples.

        Inputs:
            ast[tree.token]: The AST to convert.
        """
        self.reinit()

    def write(self, output): #pylint: disable=no-self-use
        """
        Write the rendered output content to a single string, this method automatically strips
        all double new lines.

        Inputs:
            output[tree.base.NodeBase]: The output tree created by calling render.
        """
        text = output.write()
        return re.sub(r'(\n{2,})', '\n', text, flags=re.MULTILINE)

    def process(self, parent, token):
        """
        Convert the AST defined in the token input into a output node of parent.

        Inputs:
            ast[tree.token]: The AST to convert.
            parent[tree.base.NodeBase]: A tree object that the AST shall be converted to.
        """
        try:
            func = self.__getFunction(token)
            el = func(token, parent) if func else parent

        except Exception as e: #pylint: disable=broad-except
            el = None

            title = u'ERROR:{}'.format(e.message)
            if self.translator.current:
                filename = mooseutils.colorText('{}:{}\n'.format(self.translator.current.source,
                                                                 token.info.line), 'RESET')
            else:
                filename = ''

            box = mooseutils.colorText(MooseDocs.common.box(token.info[0],
                                                            line=token.info.line,
                                                            width=100), 'LIGHT_CYAN')
            trace = mooseutils.colorText(traceback.format_exc(), 'GREY')
            msg = u'\n{}\n{}{}\n{}\n'.format(title, filename, box, trace)
            LOG.error(msg)

        if el is not None:
            for child in token.children:
                self.process(el, child)

    def _method(self, component):
        """
        Return the desired method to call on the RenderComponent object.

        Inputs:
            component[RenderComponent]: Object to use for locating desired method for renderering.
        """
        if self.METHOD is None:
            msg = "The Reader class of type {} must define the METHOD class member."
            raise exceptions.MooseDocsException(msg, type(self))
        elif not hasattr(component, self.METHOD):
            msg = "The component object {} does not have a {} method."
            raise exceptions.MooseDocsException(msg, type(component), self.METHOD)
        return getattr(component, self.METHOD)

    def __getFunction(self, token):
        """
        Return the desired function for the supplied token object.

        Inputs:
            token[tree.token]: token for which the associated RenderComponent function is desired.
        """
        return self.__functions.get(type(token), None)

class HTMLRenderer(Renderer):
    """
    Converts AST into HTML.
    """
    METHOD = 'createHTML'

    def render(self, ast):
        """
        Render the supplied AST, wrapping it in a <body> tag.
        """
        self.reinit()
        root = html.Tag(None, 'body', class_='moose-content')
        self.process(root, ast)
        return root

class MaterializeRenderer(HTMLRenderer):
    """
    Convert AST into HTML using the materialize javascript library (http://materializecss.com).
    """
    METHOD = 'createMaterialize'

    @staticmethod
    def defaultConfig():
        """
        Return the default configuration.
        """
        #config['scrollspy'] = (True, "Toggle the use of the right scrollable navigation.")
        config = HTMLRenderer.defaultConfig()
        config['breadcrumbs'] = (True, "Toggle for the breadcrumb links at the top of page.")
        config['sections'] = (True, "Group heading content into <section> tags.")
        config['collapsible-sections'] = ([None, 'open', None, None, None, None],
                                          "Collapsible setting for the six heading level " \
                                          "sections, possible values include None, 'open', and " \
                                          "'close'. Each indicates if the associated section " \
                                          "should be collapsible, if so should it be open or " \
                                          "closed initially. The 'sections' setting must be " \
                                          "True for this to operate.")
        config['navigation'] = (None, "Top bar website navigation items.")
        config['repo'] = (None, "The source code repository.")
        config['name'] = (None, "The name of the website (e.g., MOOSE)")
        config['home'] = ('/', "The homepage for the website.")
        config['scrollspy'] = (True, "Enable/disable the scrolling table of contents.")
        return config

    def __init__(self, *args, **kwargs):
        HTMLRenderer.__init__(self, *args, **kwargs)
        self.__navigation = None # Cache for navigation pages

    def update(self, **kwargs):
        """
        Update the default configuration with the supplied values. This is an override of the
        ConfigObject method and is simply modified here to the check the type of a configuration
        item.
        """
        HTMLRenderer.update(self, **kwargs)

        collapsible = self['collapsible-sections']
        if not isinstance(collapsible, list) or len(collapsible) != 6:
            msg = "The config option 'collapsible-sections' input must be a list of six entries, " \
                  "the item supplied is a {} of length {}."
            raise ValueError(msg.format(type(collapsible), len(collapsible)))

    def _method(self, component):
        """
        Fallback to the HTMLRenderer method if the MaterializeRenderer method is not located.

        Inputs:
            component[RenderComponent]: Object to use for locating desired method for renderering.
        """
        if hasattr(component, self.METHOD):
            return getattr(component, self.METHOD)
        elif hasattr(component, HTMLRenderer.METHOD):
            return getattr(component, HTMLRenderer.METHOD)
        else:
            msg = "The component object {} does not have a {} method."
            raise exceptions.MooseDocsException(msg, type(component), self.METHOD)

    def render(self, ast):
        """
        Convert supplied AST into a Materialize HTML tree.

        Inputs:
            ast[tokens.Token]: The root of the tokenized content to be converted.
        """
        Renderer.render(self, ast) # calls reinit()

        root = html.Tag(None, '!DOCTYPE html', close=False)
        html.Tag(root, 'html')

        head = html.Tag(root, 'head')
        body = html.Tag(root, 'body')
        wrap = html.Tag(body, 'div', class_='page-wrap')

        header = html.Tag(wrap, 'header')
        nav = html.Tag(html.Tag(header, 'nav'), 'div', class_='nav-wrapper container')
        main = html.Tag(wrap, 'main', class_='main')

        container = html.Tag(main, 'div', class_="container")

        # <head> content
        self.addHead(head, self.translator.current)
        self.addRepo(nav, self.translator.current)
        self.addName(nav, self.translator.current)
        self.addNavigation(nav, self.translator.current)
        self.addBreadcrumbs(container, self.translator.current)

        # Content
        row = html.Tag(container, 'div', class_="row")
        col = html.Tag(row, 'div', class_="moose-content")
        HTMLRenderer.process(self, col, ast)

        # Sections
        self.addSections(col, self.translator.current)

        if self['scrollspy']:
            col['class'] = 'col s12 m12 l10'
            toc = html.Tag(row, 'div', class_="col hide-on-med-and-down l2")
            self.addContents(toc, col, self.translator.current)
        else:
            col['class'] = 'col s12 m12 l12'

        return root

    def addHead(self, head, root_page): #pylint: disable=unused-argument,no-self-use
        """
        Add content to <head> element with the required CSS/JS for materialize.

        Inputs:
            head[html.Tag]: The <head> tag for the page being generated.
            root_page[page.PageNodeBase]: The current page being converted.
        """

        html.Tag(head, 'meta', close=False, charset="UTF-8")
        html.Tag(head, 'link', href="/contrib/materialize/materialize.min.css", type="text/css",
                 rel="stylesheet", media="screen,projection")
        html.Tag(head, 'link', href="/contrib/katex/katex.min.css", type="text/css",
                 rel="stylesheet")
        html.Tag(head, 'link', href="/css/moose.css", type="text/css", rel="stylesheet")
        html.Tag(head, 'link', href="/contrib/prism/prism.min.css", type="text/css",
                 rel="stylesheet")
        html.Tag(head, 'script', type="text/javascript", src="/contrib/jquery/jquery.min.js")
        html.Tag(head, 'script', type="text/javascript",
                 src="/contrib/materialize/materialize.min.js")

        html.Tag(head, 'script', type="text/javascript", src="/contrib/clipboard/clipboard.min.js")
        html.Tag(head, 'script', type="text/javascript", src="/contrib/prism/prism.min.js")
        html.Tag(head, 'script', type="text/javascript", src="/contrib/katex/katex.min.js")
        html.Tag(head, 'script', type="text/javascript", src="/js/init.js")

    def addContents(self, toc, content, root_page): #pylint: disable=unused-argument,no-self-use
        """
        Add the table of contents right-side bar that used scrollspy.

        Inputs:
            toc[html.Tag]: Table-of-contents <div> tag.
            content[html.Tag]: The complete html content that the TOC is going to reference.
            root_page[page.PageNodeBase]: The current page being converted.
        """

        div = html.Tag(toc, 'div', class_='toc-wrapper pin-top')
        ul = html.Tag(div, 'ul', class_='section table-of-contents')
        for node in anytree.PreOrderIter(content):
            if node.name == 'section' and node['data-section-level'] == 2:
                node['class'] = 'section scrollspy'
                li = html.Tag(ul, 'li')
                a = html.Tag(li, 'a',
                             href='#{}'.format(node['id']),
                             string=node['data-section-text'],
                             class_='tooltipped')
                a['data-delay'] = '50'
                a['data-position'] = 'left'
                a['data-tooltip'] = node['data-section-text']

    def addRepo(self, nav, root_page):
        """
        Add the repository link to the navigation bar.

        Inputs:
            nav[html.Tag]: The <div> containing the navigation for the page being generated.
            root_page[page.PageNodeBase]: The current page being converted.
        """

        repo = self.get('repo', None)
        if (repo is None) or (root_page is None):
            return

        a = html.Tag(nav, 'a', href=repo, class_='right')

        if 'github' in repo:
            img0 = root_page.findall('github-logo.png')[0]
            img1 = root_page.findall('github-mark.png')[0]

            html.Tag(a, 'img', src=img0.relativeDestination(root_page), class_='github-mark')
            html.Tag(a, 'img', src=img1.relativeDestination(root_page), class_='github-logo')

        elif 'gitlab' in repo:
            img = root_page.findall('gitlab-logo.png')[0]
            html.Tag(a, 'img', src=img.relativeDestination(root_page), class_='gitlab-logo')

    def addNavigation(self, nav, root_page):
        """
        Add the repository navigation links.

        Inputs:
            nav[html.Tag]: The <div> containing the navigation for the page being generated.
            root_page[page.PageNodeBase]: The current page being converted.
        """

        # Do nothing if navigation is not probided
        navigation = self.get('navigation', None)
        if (navigation is None) or (root_page is None):
            return

        # Locate the navigation pages
        if self.__navigation is None:
            self.__navigation = navigation
            for key1, value1 in navigation.iteritems():
                for key2, value2 in value1.iteritems():
                    node = root_page.findall(value2)
                    if node is None:
                        msg = 'Failed to locate navigation item: {}.'
                        raise exceptions.MooseDocsException(msg, value2)
                    self.__navigation[key1][key2] = node[0]

        # Build the links
        top_ul = html.Tag(nav, 'ul', id="nav-mobile", class_="right hide-on-med-and-down")
        for key1, value1 in self.__navigation.iteritems():
            id_ = uuid.uuid4()

            top_li = html.Tag(top_ul, 'li')
            a = html.Tag(top_li, 'a', class_="dropdown-button", href="#!", string=unicode(key1))
            a['data-activates'] = id_
            html.Tag(a, "i", class_='material-icons right', string=u'arrow_drop_down')

            bot_ul = html.Tag(nav, 'ul', id_=id_, class_='dropdown-content')
            for key2, node in value1.iteritems():
                bot_li = html.Tag(bot_ul, 'li')
                href = node.relativeDestination(root_page)
                a = html.Tag(bot_li, 'a', href=href, string=unicode(key2))

    def addName(self, nav, root_page): #pylint: disable=unused-argument
        """
        Add the page name to the left-hand side of the top bar.

        Inputs:
            nav[html.Tag]: The <div> containing the navigation for the page being generated.
            root_page[page.PageNodeBase]: The current page being converted.
        """

        name = self.get('name', None)
        if name:
            html.Tag(nav, 'a', class_='left', href=unicode(self.get('home', '#!')),
                     string=unicode(name))

    def addBreadcrumbs(self, container, root_page):
        """
        Inserts breadcrumb links at the top of the page.

        Inputs:
            root[tree.html.Tag]: The tag to which the breadcrumbs should be inserted.

        TODO: This is relying on hard-coded .md/.html extensions, that should not be the case.
        """

        if not (self.get('breadcrumbs', None) and root_page):
            return

        row = html.Tag(container, 'div', class_="row")
        col = html.Tag(row, 'div', class_="col hide-on-med-and-down l12")
        nav = html.Tag(col, 'nav', class_="breadcrumb-nav")
        div = html.Tag(nav, 'div', class_="nav-wrapper")

        node = root_page
        for n in node.path:
            if not n.local:
                continue
            if isinstance(n, page.DirectoryNode):
                idx = n.find('index.md', maxlevel=2)
                if idx:
                    url = os.path.relpath(n.local,
                                          os.path.dirname(node.local)).replace('.md', '.html')
                    a = html.Tag(div, 'a', href=url, class_="breadcrumb")
                    html.String(a, content=unicode(n.name))
                else:
                    span = html.Tag(div, 'span', class_="breadcrumb")
                    html.String(span, content=unicode(n.name))

            elif isinstance(n, page.FileNode) and n.name != 'index.md':
                url = os.path.relpath(n.local,
                                      os.path.dirname(node.local)).replace('.md', '.html')
                a = html.Tag(div, 'a', href=url, class_="breadcrumb")
                html.String(a, content=unicode(os.path.splitext(n.name)[0]))

    def addSections(self, container, root_page): #pylint: disable=unused-argument
        """
        Group content into <section> tags based on the heading tag.

        Inputs:
            root[tree.html.Tag]: The root tree.html node tree to add sections.read
            collapsible[list]: A list with six entries indicating the sections to create as
                               collapsible.
        """
        if not self.get('sections', False):
            return

        collapsible = self.get('collapsible-sections', False)

        section = container
        for child in section.children:
            if child.name in ('h1', 'h2', 'h3', 'h4', 'h5', 'h6'):
                level = int(child.name[-1])
                current = section.get("data-section-level", 0) # get the current section lvel

                if level == current:
                    section = html.Tag(section.parent, 'section')
                elif level > current:
                    section = html.Tag(section, 'section')
                elif level < current:
                    section = html.Tag(section.parent.parent, 'section')

                section['data-section-level'] = level
                section['data-section-text'] = child.text()
                section['id'] = uuid.uuid4()
                if 'data-details-open' in child:
                    section['data-details-open'] = child['data-details-open']

            child.parent = section

        for node in anytree.PreOrderIter(container, filter_=lambda n: n.name == 'section'):

            if 'data-details-open' in node:
                status = node['data-details-open']
            else:
                status = collapsible[node['data-section-level']-1]

            if status:
                summary = html.Tag(None, 'summary')
                node(0).parent = summary

                details = html.Tag(None, 'details', class_="moose-section-content")
                if status.lower() == 'open':
                    details['open'] = 'open'
                details.children = node.children
                summary.parent = details
                details.parent = node

                icon = html.Tag(None, 'span', class_='moose-section-icon')
                summary(0).children = [icon] + list(summary(0).children)

class LatexRenderer(Renderer):
    """
    Renderer for converting AST to LaTeX.
    """
    METHOD = 'createLatex'

    def __init__(self, *args, **kwargs):
        self._packages = set()
        Renderer.__init__(self, *args, **kwargs)

    def render(self, ast):
        """
        Built LaTeX tree from AST.
        """
        root = base.NodeBase()
        latex.Command(root, 'documentclass', string=u'book', end='\n')

        for package in self._packages:
            latex.Command(root, 'usepackage', string=package, end='\n')

        doc = latex.Environment(root, 'document')
        self.process(doc, ast)
        return root

    def addPackage(self, *args):
        """
        Add a LaTeX package to the list of packages for rendering.
        """
        self._packages.update(args)
