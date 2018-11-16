#:pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import anytree

HEADING_CACHE = dict()
def find_heading(page, ast, bookmark=u''):
    """
    Locate the first Heading token for the supplied page.

    Inputs:
        page[pages.SourceNode]: The page that will be searched.
        ast[tokens.Token]: The AST from the given page to search.
        bookmark[unicode]: The "id" for the heading.
    """

    node = HEADING_CACHE.get((page.local, bookmark), None)
    if node is not None:
        return node.copy()

    func = lambda n: (n.name == 'Heading') and (n.get('id', u'') == bookmark)
    for node in anytree.PreOrderIter(ast, filter_=func):
        HEADING_CACHE[(page.local, bookmark)] = node.copy()
        return node.copy()
