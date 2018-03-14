#pylint:disable=missing-docstring, unused-argument
import MooseDocs
from . import exceptions

def project_find(filename, mincout=None, maxcount=None, exc=exceptions.MooseDocsException):
    """
    Utility for finding files within a project based on 'git ls-files', see MooseDocs.__init__.py.

    The main use for this function is locating source files for inclusion in listings or for
    creating bottom modals with source code.

    The list of files is populated in MooseDocs.__init__.py, otherwise the list was created
    multiple times.
    """
    matches = [fname for fname in MooseDocs.PROJECT_FILES if fname.endswith(filename)]
    return matches
