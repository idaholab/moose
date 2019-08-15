#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import uuid
import logging
import json
import anytree
from MooseDocs import common
from MooseDocs.extensions import core, heading
from MooseDocs.base import components, renderers
from MooseDocs.common import exceptions
from MooseDocs.tree import html, pages

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return NavigationExtension(**kwargs)

class NavigationExtension(components.Extension):
    """
    Extension for navigation items.
    """

    @staticmethod
    def defaultConfig():
        config = components.Extension.defaultConfig()
        config['menu'] = (dict(), "Navigation items, this can either be a *.menu.md file or dict. "\
                          "The former creates a 'mega menu' and the later uses dropdowns.")
        config['search'] = (True, "Enable/disable the search bar.")
        config['home'] = ('', "The homepage for the website.")
        config['repo'] = (None, "The source code repository.")
        config['name'] = (None, "The name of the website (e.g., MOOSE)")
        config['breadcrumbs'] = (True, "Toggle for the breadcrumb links at the top of page.")
        config['sections'] = (True, "Group heading content into <section> tags.")
        config['scrollspy'] = (True, "Enable/disable the scrolling table of contents.")
        config['collapsible-sections'] = ([None, None, None, None, None, None],
                                          "Collapsible setting for the six heading level " \
                                          "sections, possible values include None, 'open', and " \
                                          "'close'. Each indicates if the associated section " \
                                          "should be collapsible, if so should it be open or " \
                                          "closed initially. The 'sections' setting must be " \
                                          "True for this to operate.")
        return config

    def extend(self, reader, renderer):
        self.requires(core, heading)

        menu = self.get('menu')
        if (not menu) and (renderer.get('navigation', None) is not None):
            msg = "The navigation setting in the MaterializeRenderer is deprecated, " \
                  "please update your code to use the 'menu' setting within the " \
                  "MooseDocs.extensions.navigation extension."
            LOG.warning(msg)
            self.update(menu=renderer.get('navigation'))

        if isinstance(renderer, renderers.MaterializeRenderer):
            renderer.addJavaScript('nav', 'js/navigation.js')

            if self.get('search', False):
                renderer.addJavaScript('fuse', "contrib/fuse/fuse.min.js")
                renderer.addJavaScript('fuse_index', "js/search_index.js")

    def initMetaData(self, page, meta):
        """Initialize the meta data to hold search index data."""
        meta.initData('search', list())

    def postTokenize(self, ast, page, meta, reader):
        """Capture the search results."""

        if not self.get('search', False):
            return

        index = []
        title = None
        for head in anytree.search.findall_by_attr(ast, 'Heading'):
            if (head['level'] == 1) and (title is None):
                title = head.text()

            id_ = head.get('id')
            if id_ == u'':
                id_ = head.text('-').lower()
            index.append(dict(title=title, text=head.text(), bookmark=id_))
        meta.getData('search').extend(index)

    def postExecute(self, content):
        """Build the JSON file containing the index data."""
        dest = self.translator.get('destination') #pylint: disable=no-member
        home = self.get('home')
        iname = os.path.join(dest, 'js', 'search_index.js')
        items = []

        for page in content:
            meta = self.translator.getMetaData(page, 'search')
            if meta is None:
                continue
            location = page.destination.replace(dest, home)
            for data in meta:
                url = '{}#{}'.format(location, data['bookmark'])
                items.append(dict(title=data['title'], text=data['text'], location=url))

        if not os.path.isdir(os.path.dirname(iname)):
            os.makedirs(os.path.dirname(iname))
        common.write(iname, 'var index_data = {};'.format(json.dumps(items)))

    def postRender(self, result, page, meta, renderer):
        """Called after rendering is complete."""

        if not isinstance(renderer, renderers.MaterializeRenderer):
            return

        root = result.root
        header = anytree.search.find_by_attr(root, 'header')
        nav = html.Tag(html.Tag(header, 'nav'), 'div', class_='nav-wrapper container')
        container = anytree.search.find_by_attr(root, 'main').children[0]

        row = container(0)
        col = container(0)(0)

        self._addTopNavigation(nav, page)
        self._addSideNavigation(nav, page)

        if self.get('breadcrumbs'):
            self._addBreadcrumbs(container, page)

        if self.get('sections'):
            self._addSections(col, page)

        if self.get('scrollspy'):
            col.addClass('col', 's12', 'm12', 'l10')
            toc = html.Tag(row, 'div', class_="col hide-on-med-and-down l2")
            self._addContents(toc, col, page)

        else:
            col.addClass('col', 's12', 'm12', 'l12')

        if self.get('search'):
            self._addSearch(nav, page)

        repo = self.get('repo', None)
        if repo is not None:
            self._addRepo(nav, page)

        head = anytree.search.find_by_attr(root, 'head')
        self._addTitle(head, root, page)
        self._addName(nav, page)

    def _addName(self, nav, page): #pylint: disable=unused-argument
        """
        Add the page name to the left-hand side of the top bar.

        Inputs:
            nav[html.Tag]: The <div> containing the navigation for the page being generated.
            page[page.PageNodeBase]: The current page being converted.
        """
        name = self.get('name', None)
        if name is not None:
            a = html.Tag(None, 'a', class_='left moose-logo hide-on-med-and-down',
                         href=str(self.get('home', '#!')),
                         string=str(name))
            nav.insert(0, a)

    def _addRepo(self, nav, page): #pylint: disable=no-self-use
        """
        Add the repository link to the navigation bar.

        Inputs:
            nav[html.Tag]: The <div> containing the navigation for the page being generated.
            page[page.PageNodeBase]: The current page being converted.
        """
        repo = self.get('repo')
        a = html.Tag(None, 'a', href=repo, class_='right')

        if 'github' in repo:
            img0 = self.translator.findPage('github-logo.png')
            img1 = self.translator.findPage('github-mark.png')

            html.Tag(a, 'img', src=img0.relativeDestination(page), class_='github-mark')
            html.Tag(a, 'img', src=img1.relativeDestination(page), class_='github-logo')

        elif 'gitlab' in repo:
            img = self.translator.findPage('gitlab-logo.png')
            html.Tag(a, 'img', src=img.relativeDestination(page), class_='gitlab-logo')

        nav.insert(0, a)

    def _addTitle(self, head, root, page): #pylint: disable=unused-argument
        """
        Add content to <title> tag.

        Inputs:
            head[html.Tag]: The <head> tag for the page being generated.
            ast[tokens.Token]: The root node for the AST.
            page[page.PageNodeBase]: The current page being converted.
        """

        # Locate h1 heading, if it is found extract the rendered text
        h = heading.find_heading(self.translator, page)
        page_name = h.text() if h else page.name
        name = self.get('name', None)
        if name is not None:
            html.Tag(head, 'title', string=u'{}|{}'.format(page_name, self.get('name')))
        else:
            html.Tag(head, 'title', string=str(page_name))

    def _addSearch(self, parent, page):

        # Search button
        btn = html.Tag(parent, 'a', class_="modal-trigger", href="#moose-search")
        html.Tag(btn, 'i', string=u'search', class_="material-icons")

        # Search modal
        div = html.Tag(anytree.search.find_by_attr(parent.root, 'header'), 'div',
                       id_="moose-search",
                       class_="modal modal-fixed-footer moose-search-modal")
        container = html.Tag(div, 'div',
                             class_="modal-content container moose-search-modal-content")
        row = html.Tag(container, 'div', class_="row")
        col = html.Tag(row, 'div', class_="col l12")
        box_div = html.Tag(col, 'div', class_="input-field")
        html.Tag(box_div, 'input',
                 type_='text',
                 id_="moose-search-box",
                 onkeyup="mooseSearch()",
                 placeholder=str(self.get('home')))
        result_wrapper = html.Tag(row, 'div')

        html.Tag(result_wrapper, 'div', id_="moose-search-results", class_="col s12")
        footer = html.Tag(div, 'div', class_="modal-footer")
        html.Tag(footer, 'a', href='#!', class_="modal-action modal-close btn-flat",
                 string=u'Close')

    def _addTopNavigation(self, parent, page):
        """Create navigation in the top bar."""
        ul = html.Tag(parent, 'ul', class_="right hide-on-med-and-down")
        self._createNavigation(ul, page)

    def _addSideNavigation(self, parent, page):
        """Adds the hamburger menu for small screens."""
        id_ = uuid.uuid4()

        a = html.Tag(parent, 'a', href='#', class_='sidenav-trigger')
        a['data-target'] = id_
        html.Tag(a, 'i', class_="material-icons", string=u'menu')

        ul = html.Tag(parent, 'ul', class_="sidenav", id_=id_)
        self._createNavigation(ul, page, mega=False)

    def _addBreadcrumbs(self, container, page): #pylint: disable=no-self-use
        """
        Inserts breadcrumb links at the top of the page.

        Inputs:
            root[tree.html.Tag]: The tag to which the breadcrumbs should be inserted.

        TODO: This is relying on hard-coded .md/.html extensions, that should not be the case.
        """

        row = html.Tag(None, 'div', class_="row")
        col = html.Tag(row, 'div', class_="col hide-on-med-and-down l12")
        nav = html.Tag(col, 'nav', class_="breadcrumb-nav")
        div = html.Tag(nav, 'div', class_="nav-wrapper")

        container.insert(0, row)

        parts = page.local.split(os.sep)
        for i in range(1, len(parts)):
            current = self.translator.findPage(lambda p: p.local == os.path.join(*parts[:i])) #pylint: disable=cell-var-from-loop

            if isinstance(current, pages.Directory):
                idx = self.translator.findPages(os.path.join(current.local, 'index.md'))
                if idx:
                    url = os.path.relpath(current.local,
                                          os.path.dirname(page.local)).replace('.md', '.html')
                    a = html.Tag(div, 'a', href=url,
                                 class_="breadcrumb",
                                 string=str(current.name))
                else:
                    span = html.Tag(div, 'span', class_="breadcrumb")
                    html.String(span, content=str(current.name))

            elif isinstance(current, pages.File) and current.name != 'index.md':
                url = os.path.relpath(current.local,
                                      os.path.dirname(page.local)).replace('.md', '.html')
                a = html.Tag(div, 'a', href=url, class_="breadcrumb")
                html.String(a, content=str(os.path.splitext(current.name)[0]))

        if not page.local.endswith('index.md'):
            html.Tag(div, 'a', href='#', class_="breadcrumb",
                     string=str(os.path.splitext(page.name)[0]))

    def _addSections(self, container, page): #pylint: disable=unused-argument
        """
        Group content into <section> tags based on the heading tag.

        Inputs:
            root[tree.html.Tag]: The root tree.html node tree to add sections.read
            collapsible[list]: A list with six entries indicating the sections to create as
                               collapsible.
        """
        collapsible = self.get('collapsible-sections')
        if isinstance(collapsible, str):
            collapsible = eval(collapsible)

        section = container
        for child in section.children:
            if child.name in ('h1', 'h2', 'h3', 'h4', 'h5', 'h6'):
                level = int(child.name[-1])
                current = section.get("data-section-level", 0) # get the current section level

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

    def _addContents(self, toc, content, page): #pylint: disable=unused-argument,no-self-use
        """
        Add the table of contents right-side bar that used scrollspy.

        Inputs:
            toc[html.Tag]: Table-of-contents <div> tag.
            content[html.Tag]: The complete html content that the TOC is going to reference.
            page[page.PageNodeBase]: The current page being converted.
        """

        div = html.Tag(toc, 'div', class_='toc-wrapper pin-top')
        ul = html.Tag(div, 'ul', class_='section table-of-contents')
        for node in anytree.PreOrderIter(content):
            if node.get('data-section-level', None) == 2:
                node.addClass('scrollspy')
                li = html.Tag(ul, 'li')
                a = html.Tag(li, 'a',
                             href='#{}'.format(node['id']),
                             string=node.get('data-section-text', 'Unknown...'),
                             class_='tooltipped')
                a['data-position'] = 'left'
                a['data-tooltip'] = node['data-section-text']

    def _createNavigation(self, ul, page, mega=True):
        """Helper for creating navigation lists."""

        for key, value in self.get('menu', dict()).items(): #pylint: disable=no-member

            li = html.Tag(ul, 'li')
            if isinstance(value, str) and value.endswith('menu.md') and mega:
                li['class'] = 'moose-mega-menu-trigger'
                a = html.Tag(li, 'a', string=str(key))
                html.Tag(a, 'i', class_='material-icons right', string=u'arrow_drop_down')
                self._addMegaMenu(li, value, page)

            elif isinstance(value, str):
                href = value if value.startswith('http') else self._findPath(page, value)
                html.Tag(li, 'a', href=href, string=str(key))

            elif isinstance(value, dict):
                id_ = uuid.uuid4()
                a = html.Tag(li, 'a', class_='dropdown-trigger', href='#!', string=str(key))
                a['data-target'] = id_
                a['data-constrainWidth'] = 'false'
                html.Tag(a, 'i', class_='material-icons right', string=u'arrow_drop_down')
                self._buildDropdown(ul.parent.parent, page, id_, value)

            else:
                msg = 'Top-level navigation items must be string or dict.'
                LOG.error(msg)
                raise exceptions.MooseDocsException(msg)

    def _addMegaMenu(self, parent, filename, page):
        """Create a "mega menu" by parsing the *.menu.md file."""

        id_ = uuid.uuid4()
        header = anytree.search.find_by_attr(parent.root, 'header')
        div = html.Tag(header, 'div', id_=id_)
        div.addClass('moose-mega-menu-content')
        parent['data-target'] = id_

        wrap = html.Tag(div, 'div', class_='moose-mega-menu-wrapper')
        node = self.translator.findPage(filename)
        ast = self.translator.getSyntaxTree(node) #pylint: disable=no-member
        self.translator.renderer.render(wrap, ast, page) #pylint: disable=no-member

    def _buildDropdown(self, parent, page, tag_id, items):
        """Creates sublist for dropdown navigation."""
        ul = html.Tag(parent, 'ul', id_=tag_id, class_='dropdown-content')
        for key, value in items.items():
            li = html.Tag(ul, 'li')
            href = value if value.startswith('http') else self._findPath(page, value)
            html.Tag(li, 'a', href=href, string=str(key))

    def _findPath(self, page, path):
        """Locates page based on supplied path."""
        node = self.translator.findPage(path.lstrip('/'), exact=path.startswith('/'))
        if node is None:
            msg = 'Failed to locate navigation item: {}.'.format(path)
            LOG.error(msg)
            raise exceptions.MooseDocsException(msg)
        return node.relativeDestination(page)
