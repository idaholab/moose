import sys
import os
import re
import subprocess

import clang.cindex
if 'MOOSE_CLANG_LIB' not in os.environ:
    raise EnvironmentError("Using the MooseSourceParser requires setting 'MOOSE_CLANG_LIB' environment variable to point to the clang library.")
clang.cindex.Config.set_library_path(os.getenv('MOOSE_CLANG_LIB'))

class MooseSourceParser(object):
    """
    An object for parsing MOOSE source code.

    Args:
        app_path[str]: The path that contains the application Makefile (needed for extracting includes).
    """


    def __init__(self, app_path):

        # Check that the supplied path has a Makefile (for getting includes)
        if not os.path.exists(os.path.join(app_path, 'Makefile')):
            #TODO: Make this a MooseException and log the exception and also check that the make file os one from MOOSE
            print 'The supplied application directory does not contain a Makefile:', app_path
            return

        # Extract the includes from the Makefile
        self._includes = self.includes(app_path)

    def parse(self, filename):
        """
        Parse the supplied C/h file with clang.

        Args:
            filename[str]: The filename to parse.
        """

        # Check that the supplied file exists
        if not os.path.exists(filename):
            #TODO: Proper exception and logging
            print 'The supplied source/header file does not exist:', filename
            return

        # Build the flags to pass to clang
        includes = ['-x', 'c++', '-std=c++11']
        includes += self._includes

        # Build clang translation unit
        index = clang.cindex.Index.create()
        self._translation_unit = index.parse(filename, includes)

    @staticmethod
    def includes(app_path):
        """
        Returns the includes by running 'make echo_include' for an application.

        Args:
            app_path[str]: A valid moose application or directory with a MOOSE Makefile (e.g., framework).
        """

        p = subprocess.Popen(['make', 'echo_include'], cwd=app_path, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output, err = p.communicate()
        for match in re.finditer(r'-I(.*?)\s', output):
            yield match.group(0).strip().strip('\n')

    def method(self, name):
        """
        Retrieve a class declaration and definition by name.

        Args:
            name[str]: The name of the method to extract.

        Returns:
           decl[str], defn[str]: A string containing the declaration and definition of the desired method.
        """

        decl = None
        defn = None
        cursors = self.find(clang.cindex.CursorKind.CXX_METHOD, name=name)
        for c in cursors:
            if c.is_definition():
                defn = self.content(c)
            else:
                decl = self.content(c)

        return decl, defn

    def dump(self, cursor=None, level = 0, **kwargs):
        """
        A tool for dumping the cursor tree.
        """
        if cursor == None:
            cursor = self._translation_unit.cursor,
        recursive = kwargs.pop('recursive', True)
        for c in cursor.get_children():
            print ' '*4*level, c.kind, c.spelling, c.extent.start.file, c.extent.start.line
            if recursive and c.get_children():
                self.dump(c, level+1)

    @staticmethod
    def content(cursor):
        source_range = cursor.extent
        fid = open(source_range.start.file.name, 'r')
        content = fid.read()[source_range.start.offset:source_range.end.offset]
        fid.close()
        return content


    def find(self, kind, **kwargs):
        """
        Locate the clang.cindex.Cursor object(s). (public)

        Args:
            kind[int]: The type of cursor (see clang.cindex.py) to locate.

        Kwargs:
            name[str]: The name of the cursor to return (i.e., Cursor.spelling)
            definition[bool]: Only include items with 'is_definition' set to true.

        Returns:
            A list of all cursors matching the kind and optionally the name.
        """

        name = kwargs.pop('name', None)
        defn = kwargs.pop('definition', False)

        for cursor in self._translation_unit.cursor.walk_preorder():
            if (hasattr(cursor, 'kind')) and (cursor.kind == kind) and (name == None or cursor.spelling == name):
                #print cursor.extent.start.file
                yield cursor




if __name__ == '__main__':

    src = '/Users/slauae/projects/moose/framework/src/kernels/Diffusion.C'

    parser = MooseSourceParser('/Users/slauae/projects/moose/framework')
    parser.parse(src)
    decl, defn = parser.method('computeQpResidual')
    print decl, defn
