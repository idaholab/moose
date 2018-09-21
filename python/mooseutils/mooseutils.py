#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from __future__ import print_function
import os
import re
import collections
import math
import difflib
import multiprocessing
import subprocess

def colorText(string, color, **kwargs):
    """
    A function for coloring text.

    Args:
        string[str]: The string to color.
        color[str]: The color to use (see color_codes variable).

    Kwargs:
        html[bool]: If true html colored text is returned.
        colored[bool]: When false the coloring is not applied.
    """
    # Get the properties
    html = kwargs.pop('html', False)
    code = kwargs.pop('code', True)
    colored = kwargs.pop('colored', True)

    # ANSI color codes for colored terminal output
    color_codes = dict(RESET='\033[0m',
                       BOLD='\033[1m',
                       RED='\033[31m',
                       GREEN='\033[32m',
                       YELLOW='\033[33m',
                       BLUE='\033[34m',
                       MAGENTA='\033[35m',
                       CYAN='\033[36m',
                       GREY='\033[90m',
                       LIGHT_RED='\033[91m',
                       LIGHT_GREEN='\033[92m',
                       LIGHT_YELLOW='\033[93m',
                       LIGHT_BLUE='\033[94m',
                       LIGHT_MAGENTA='\033[95m',
                       LIGHT_CYAN='\033[96m',
                       LIGHT_GREY='\033[37m')
    if code:
        color_codes['GREEN'] = '\033[32m'
        color_codes['CYAN']  = '\033[36m'
        color_codes['MAGENTA'] = '\033[35m'

    if colored and not (os.environ.has_key('BITTEN_NOCOLOR') and os.environ['BITTEN_NOCOLOR'] == 'true'):
        if html:
            string = string.replace('<r>', color_codes['BOLD']+color_codes['RED'])
            string = string.replace('<c>', color_codes['BOLD']+color_codes['CYAN'])
            string = string.replace('<g>', color_codes['BOLD']+color_codes['GREEN'])
            string = string.replace('<y>', color_codes['BOLD']+color_codes['YELLOW'])
            string = string.replace('<b>', color_codes['BOLD'])
            string = re.sub(r'</[rcgyb]>', color_codes['RESET'], string)
        else:
            string = color_codes[color] + string + color_codes['RESET']
    elif html:
        string = re.sub(r'</?[rcgyb]>', '', string)    # stringip all "html" tags

    return string

def str2bool(string):
    """
    A function for converting string to boolean.

    Args:
        string[str]: The text to convert (e.g., 'true' or '1')
    """
    string = string.lower()
    if string is 'true' or string is '1':
        return True
    else:
        return False

def find_moose_executable(loc, **kwargs):
    """

    Args:
        loc[str]: The directory containing the MOOSE executable.

    Kwargs:
        methods[list]: (Default: ['opt', 'oprof', 'dbg', 'devel']) The list of build types to consider.
        name[str]: (Default: opt.path.basename(loc)) The name of the executable to locate.
        show_error[bool]: (Default: True) Display error messages.
    """

    # Set the methods and name local variables
    if 'METHOD' in os.environ:
        methods = [os.environ['METHOD']]
    else:
        methods = ['opt', 'oprof', 'dbg', 'devel']
    methods = kwargs.pop('methods', methods)
    name = kwargs.pop('name', os.path.basename(loc))
    show_error = kwargs.pop('show_error', True)

    # Handle 'combined' and 'tests'
    if os.path.isdir(loc):
        if name == 'test':
            name = 'moose_test'

    # Check that the location exists and that it is a directory
    exe = None
    loc = os.path.abspath(loc)
    if not os.path.isdir(loc):
        if show_error:
            print('ERROR: The supplied path must be a valid directory:', loc)

    # Search for executable with the given name
    else:
        for method in methods:
            exe_name = os.path.join(loc, name + '-' + method)
            if os.path.isfile(exe_name):
                exe = exe_name
            break

    # Returns the executable or error code
    if (exe is None) and show_error:
        print('ERROR: Unable to locate a valid MOOSE executable in directory:', loc)
    return exe

def runExe(app_path, args):
    """
    A function for running an application.

    Args:
        app_path[str]: The application to execute.
        args[list]: The arguments to pass to the executable.
    """
    import subprocess

    popen_args = [str(app_path)]
    if isinstance(args, str):
        popen_args.append(args)
    else:
        popen_args.extend(args)

    proc = subprocess.Popen(popen_args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    data = proc.communicate()
    stdout_data = data[0].decode("utf-8")
    return stdout_data

def check_configuration(packages):
    """
    Check that the supplied packages exist.

    Return:
        [int]: 0 = Success; 1 = Missing package(s)
    """
    missing = []
    for package in packages:
        try:
            __import__(package)
        except ImportError:
            missing.append(package)

    if missing:
        print("The following packages are missing but required:")
        for m in missing:
            print(' '*4, '-', m)
        print('It may be possible to install them using "pip", but you likely need to ' \
              'the MOOSE environment package on your system.\n')
        print('Using pip:\n    pip install package-name-here --user')
        return 1

    return 0

def touch(fname):
    """
    Touch a file so to update modified time.
    """
    with open(fname, 'a'):
        os.utime(fname, None)


def gold(filename):
    """
    Get the gold filename corresponding to a filename.
    """
    if not filename:
        return None

    if not os.path.exists(filename):
        return None

    fn = os.path.basename(filename)
    dn = os.path.dirname(filename)
    gold = os.path.join(dn, 'gold', fn)
    if os.path.exists(gold):
        return gold
    return None

def unique_list(output, input):
    """
    Insert items into list, but only if they are unique.
    """
    for item in input:
        if item not in output:
            output.append(item)

def make_chunks(local, num=multiprocessing.cpu_count()):
    """
    Divides objects into equal size containers for parallel execution.

    Input:
        local[list]: A list of objects to break into chunks.
        num[int]: The number of chunks (defaults to number of threads available)
    """
    num = int(math.ceil(len(local)/float(num)))
    for i in range(0, len(local), num):
        yield local[i:i + num]

def check_file_size(base=os.getcwd(), size=1, ignore=None):
    """
    Check the supplied directory for files greater then a prescribed size.

    Input:
        base[str]: The root directory to recursively search
        size[int]: The size in MiB to check against
        ignore[pattern]: A glob pattern to ignore
    """

    # Define the size in bytes
    size = size * 1024.**2

    # Define ignore sets
    if ignore is None:
        ignore = set()

    # Search for files that are too large
    FileInfo = collections.namedtuple('FileInfo', 'name size')
    output = []
    for root, _, files in os.walk(base, topdown=False):
        for name in files:
            filename = os.path.join(root, name)
            if filename in ignore:
                continue
            result = os.stat(filename)
            if result.st_size > size:
                output.append(FileInfo(name=filename, size=result.st_size/(1024.**2)))
    return output

def camel_to_space(text):
    """
    Converts the supplied camel case text to space separated words.
    """
    out = []
    index = 0
    for match in re.finditer(r'(?<=[a-z])(?=[A-Z])', text):
        out.append(text[index:match.start(0)])
        index = match.start(0)
    out.append(text[index:])
    return ' '.join(out)

def text_diff(text, gold):
    """
    Helper for creating nicely formatted text diff message.

    Inputs:
        text[list|str]: A list of strings or single string to compare.
        gold[list|str]: The "gold" standard to which the first arguments is to be compared against.
    """

    # Convert to line
    if isinstance(text, (str, unicode)):
        text = text.splitlines(True)
    if isinstance(gold, (str, unicode)):
        gold = gold.splitlines(True)

    # Perform diff
    result = list(difflib.ndiff(gold, text))
    n = len(max(result, key=len))
    msg = "\nThe supplied text differs from the gold as follows:\n{0}\n{1}\n{0}" \
         .format('~'*n, '\n'.join(result).encode('utf-8'))
    return msg

def unidiff(out, gold, **kwargs):
    """
    Perform a 'unified' style diff between the two supplied files.

    Inputs:
        out[str]: The name of the file in question.
        gold[str]: The "gold" standard for the supplied file.
        color[bool]: When True color is applied to the diff.
        num_lines[int]: The number of lines to include with the diff (default: 3).
    """

    with open(out, 'r') as fid:
        out_content = fid.read()
    with open(gold, 'r') as fid:
        gold_content = fid.read()

    return text_unidiff(out_content, gold_content, out_fname=out, gold_fname=gold, **kwargs)

def text_unidiff(out_content, gold_content, out_fname=None, gold_fname=None, color=True, num_lines=3):
    """
    Perform a 'unified' style diff between the two supplied files.

    Inputs:
        out_content[str]: The content in question.
        gold_content[str]: The "gold" standard for the supplied content.
        color[bool]: When True color is applied to the diff.
        num_lines[int]: The number of lines to include with the diff (default: 3).

    """
    diff = []
    for line in difflib.unified_diff(out_content.splitlines(True),
                                     gold_content.splitlines(True),
                                     fromfile=out_fname,
                                     tofile=gold_fname, n=num_lines):

        if color:
            if line.startswith('-'):
                line = colorText(line, 'RED')
            elif line.startswith('+'):
                line = colorText(line, 'GREEN')
            elif line.startswith('@'):
                line = colorText(line, 'CYAN')


        diff.append(line)
    return ''.join(diff)


def git_ls_files(working_dir=os.getcwd()):
    """
    Return a list of files via 'git ls-files'.
    """
    out = set()
    for fname in subprocess.check_output(['git', 'ls-files'], cwd=working_dir).split('\n'):
        out.add(os.path.abspath(os.path.join(working_dir, fname)))
    return out

def git_root_dir(working_dir=os.getcwd()):
    """
    Return the top-level git directory by running 'git rev-parse --show-toplevel'.
    """
    try:
        return subprocess.check_output(['git', 'rev-parse', '--show-toplevel'],
                                       cwd=working_dir,
                                       stderr=subprocess.STDOUT).strip('\n')
    except subprocess.CalledProcessError:
        print("The supplied directory is not a git repository: {}".format(working_dir))
    except OSError:
        print("The supplied directory does not exist: {}".format(working_dir))
