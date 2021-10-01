#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import sys
import os
import re
import collections
import difflib
import multiprocessing
import subprocess
import time
import cProfile as profile
import pstats
try:
    from io import StringIO
except ImportError:
    from io import StringIO

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
    colored = kwargs.pop('colored', True)

    # ANSI color codes for colored terminal output
    color_codes = dict(RESET='\033[0m',
                       BOLD='\033[1m',
                       DIM='\033[2m',
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

    if colored and html:
        string = string.replace('<r>', color_codes['BOLD']+color_codes['RED'])
        string = string.replace('<c>', color_codes['BOLD']+color_codes['CYAN'])
        string = string.replace('<g>', color_codes['BOLD']+color_codes['GREEN'])
        string = string.replace('<y>', color_codes['BOLD']+color_codes['YELLOW'])
        string = string.replace('<b>', color_codes['BOLD'])
        string = re.sub(r'</[rcgyb]>', color_codes['RESET'], string)
    elif colored and not html:
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
    if string == 'true' or string == '1':
        return True
    else:
        return False

def find_moose_executable(loc, **kwargs):
    """

    Args:
        loc[str]: The directory containing the MOOSE executable.

    Kwargs:
        methods[list]: (Default: ['opt', 'oprof', 'dbg', 'devel']) The list of build types to consider.
        name[str]: The name of the executable to locate, if not provided it will infer it from
                   a Makefile or the supplied directory
        show_error[bool]: (Default: True) Display error messages.
    """

    # Set the methods and name local variables
    if 'METHOD' in os.environ:
        methods = [os.environ['METHOD']]
    else:
        methods = ['opt', 'oprof', 'dbg', 'devel']
    methods = kwargs.pop('methods', methods)
    name = kwargs.pop('name', None)

    # If the 'name' is not provided first look for a Makefile with 'APPLICATION_NAME...' if
    # that is not found use the name of the directory
    if name is None:
        makefile = os.path.join(loc, 'Makefile')
        if os.path.isfile(makefile):
            with open(makefile, 'r') as fid:
                content = fid.read()
            matches = re.findall(r'APPLICATION_NAME\s*[:=]+\s*(?P<name>.+)$', content, flags=re.MULTILINE)
            name = matches[-1] if matches else None


    loc = os.path.abspath(loc)
    # If we still don't have a name, let's try the tail of the path
    if name is None:
        name = os.path.basename(loc)

    show_error = kwargs.pop('show_error', True)
    exe = None

    # Check that the location exists and that it is a directory
    if not os.path.isdir(loc):
        if show_error:
            print('ERROR: The supplied path must be a valid directory:', loc)

    # Search for executable with the given name
    else:
        # Handle 'tests'
        if name == 'test':
            name = 'moose_test'

        for method in methods:
            exe_name = os.path.join(loc, name + '-' + method)
            if os.path.isfile(exe_name):
                exe = exe_name
            break

    # Returns the executable or error code
    if (exe is None) and show_error:
        print('ERROR: Unable to locate a valid MOOSE executable in directory:', loc)
    return exe

def find_moose_executable_recursive(loc=os.getcwd(), **kwargs):
    """
    Locate a moose executable in the current directory or any parent directory.

    Inputs: see 'find_moose_executable'
    """
    loc = loc.split(os.path.sep)
    for i in range(len(loc), 0, -1):
        current = os.path.sep + os.path.join(*loc[0:i])
        executable = find_moose_executable(current, show_error=False)
        if executable is not None:
            break
    return executable

def run_executable(app_path, *args, mpi=None, suppress_output=False):
    """
    A function for running an application.
    """
    import subprocess
    if mpi and isinstance(mpi, int):
        cmd = ['mpiexec', '-n', str(mpi), app_path]
    else:
        cmd = [app_path]
    cmd += args

    kwargs = dict(encoding='utf-8')
    if suppress_output:
        kwargs['stdout'] = subprocess.DEVNULL
        kwargs['stderr'] = subprocess.DEVNULL
    return subprocess.call(cmd, **kwargs)

def runExe(app_path, args):
    """
    A function for running an application (w/o output).

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

def check_configuration(packages, message=True):
    """
    Check that the supplied packages exist.

    Return:
        [list]: A list of missing packages.
    """
    missing = []
    for package in packages:
        try:
            __import__(package)
        except ImportError:
            missing.append(package)

    if missing and message:
        msg = "The following packages are missing but required: {0}\n"
        msg += "These packages are included in the MOOSE environment package, but it may also\n"
        msg += "to install them using 'pip':\n"
        msg += "    pip install {0} --user')"
        print(msg.format(', '.join(missing)))

    return missing

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
    k, m = divmod(len(local), num)
    return (local[i * k + min(i, m):(i + 1) * k + min(i + 1, m)] for i in range(num))

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
    if isinstance(text, str):
        text = text.splitlines(True)
    if isinstance(gold, str):
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

    lines = difflib.unified_diff(gold_content.splitlines(True),
                                 out_content.splitlines(True),
                                 fromfile=gold_fname,
                                 tofile=out_fname, n=num_lines)

    diff = []
    for line in list(lines):
        if color:
            if line.startswith('-'):
                line = colorText(line, 'RED')
            elif line.startswith('+'):
                line = colorText(line, 'GREEN')
            elif line.startswith('@'):
                line = colorText(line, 'CYAN')
        diff.append(line)

    return ''.join(diff)

def list_files(working_dir=os.getcwd()):
    """
    Return a set of files, recursively, for the supplied directory.
    """
    out = set()
    for root, dirs, filenames in os.walk(working_dir):
        for fname in filenames:
            out.add(os.path.join(root, fname))
    return out

def run_time(function, *args, **kwargs):
    """Run supplied function with duration timing."""
    start = time.time()
    out = function(*args, **kwargs)
    return time.time() - start

def run_profile(function, *args, **kwargs):
    """Run supplied function with python profiler."""
    pr = profile.Profile()
    start = time.time()
    out = pr.runcall(function, *args, **kwargs)
    print('Total Time:', time.time() - start)
    s = StringIO()
    ps = pstats.Stats(pr, stream=s).sort_stats('tottime')
    ps.print_stats()
    print(s.getvalue())
    return out

def shellCommand(command, cwd=None):
    """
    Run a command in the shell.
    We can ignore anything on stderr as that can potentially mess up the output
    of an otherwise successful command.
    """
    with open(os.devnull, 'w') as devnull:
        p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=devnull, cwd=cwd)
        p.wait()
        retcode = p.returncode
        if retcode != 0:
            raise Exception("Exception raised while running the command: %s in directory %s" % (command, cwd))

        return p.communicate()[0].decode()

def check_output(cmd, **kwargs):
    """Get output from a process"""
    kwargs.setdefault('check', True)
    kwargs.setdefault('stdout', subprocess.PIPE)
    kwargs.setdefault('stderr', subprocess.STDOUT)
    kwargs.setdefault('encoding', 'utf-8')
    return subprocess.run(cmd, **kwargs).stdout

def generate_filebase(string, replace='_', lowercase=True):
    """
    Convert the supplied string to a valid filename without spaces.
    """
    if lowercase:
        string = string.lower()
    string = re.sub(r'([\/\\\?%\*:\|\"<>\. ]+)', replace, string)
    return string

def recursive_update(d, u):
    """Recursive update nested dict(), see https://stackoverflow.com/a/3233356/1088076"""
    for k, v in u.items():
        d[k] = recursive_update(d.get(k, dict()), v) if isinstance(v, dict) else v
    return d

def fuzzyEqual(test_value, true_value, tolerance):
    return abs(test_value - true_value) / abs(true_value) < tolerance

def fuzzyAbsoluteEqual(test_value, true_value, tolerance):
    return abs(test_value - true_value) < tolerance
