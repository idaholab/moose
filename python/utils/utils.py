import os
import re
import errno

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
  color_codes = {'RESET':'\033[0m','BOLD':'\033[1m','RED':'\033[31m','GREEN':'\033[35m','CYAN':'\033[34m','YELLOW':'\033[33m','MAGENTA':'\033[32m'}
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
    """

    # Set the methods and name local varaiables
    methods = kwargs.pop('methods', ['opt', 'oprof', 'dbg', 'devel'])
    name = kwargs.pop('name', os.path.basename(loc))

    # Handle 'combined' and 'tests'
    if os.path.isdir(loc):
        if name == 'combined':
            name = 'modules'
        elif name == 'tests':
            name = 'moose_test'

    # Check that the location exists and that it is a directory
    loc = os.path.abspath(loc)
    if not os.path.isdir(loc):
        print 'ERROR: The supplied path must be a valid directory:', loc
        return errno.ENOTDIR

    # Search for executable with the given name
    exe = errno.ENOENT
    for method in methods:
        exe = os.path.join(loc, name + '-' + method)
        if os.path.isfile(exe):
            break

    # Returns the executable or error code
    if not errno.ENOENT:
        print 'ERROR: Unable to locate a valid MOOSE executable in directory'
    return exe

def runExe(app_path, args):
    """
    A function for running an application.

    Args:
        app_path[str]: The application to execute.
        args[list]: The arguuments to pass to the executable.
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
        except ImportError, e:
            missing.append(package)

    if missing:
        print "The following packages are missing but required:"
        for m in missing:
            print ' '*4, '-', m
        return 1

    return 0
