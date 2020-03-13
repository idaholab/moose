#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Tool for adding registerExecFlags(...) method to App.C/h files.
"""
import os
import sys
import re
import glob

def update_app(source):
    """
    Add the necessary code to the App.C/h files for registering flags.
    """

    # Update include file
    include = source.replace('src/', 'include/')[:-1] + 'h'
    with open(include, 'r') as fid:
        content = fid.read()

    regex = r'(( *)static +void +associateSyntax\(.*?\);)'
    sub = r'\1\n\2static void registerExecFlags(Factory & factory);'
    content = re.sub(regex, sub, content, count=1)
    with open(include, 'w') as fid:
        fid.write(content)

    # Update source file
    with open(source, 'r') as fid:
        content = fid.read()

    regex = r'(( *)(\w+)App::associateSyntax\(.*?\);)'
    sub = r'\1\n\n\2Moose::registerExecFlags(_factory);\n'\
          r'\2\3App::registerExecFlags(_factory);'
    content = re.sub(regex, sub, content, count=1)

    app = re.search(r'/(\w+)App\.C', source).group(1)
    sub = '\n// External entry point for dynamic execute flag registration\n' \
          'extern "C" void\n' \
          '{0}App__registerExecFlags(Factory & factory)\n' \
          '{{\n  {0}App::registerExecFlags(factory);\n}}\n' \
          'void\n' \
          '{0}App::registerExecFlags(Factory & /*factory*/)\n{{\n}}\n'
    content += sub.format(app)

    if app.endswith('Test'):
        regex = r'(( *)(\w+)TestApp::associateSyntax\(.*?\);)'
        sub = r'\1\n\2\3TestApp::registerExecFlags(_factory);'

    with open(source, 'w') as fid:
        fid.write(content)

if __name__ == '__main__':

    # Update app
    src = glob.glob(os.path.join('src', 'base', '*App.C'))
    if len(src) != 1:
        print "Unable to locate your *App.C file, this script should be executed from the root " \
              "directory of your repository."
        sys.exit(1)

    update_app(src[0])

    # Update test app
    src = glob.glob(os.path.join('test', 'src', 'base', '*App.C'))
    if len(src) > 0:
        update_app(src[0])
