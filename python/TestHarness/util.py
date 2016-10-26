import platform, os, re
from subprocess import *
from time import strftime, gmtime, ctime, localtime, asctime
from utils import colorText

TERM_COLS = 110

LIBMESH_OPTIONS = {
  'mesh_mode' :    { 're_option' : r'#define\s+LIBMESH_ENABLE_PARMESH\s+(\d+)',
                     'default'   : 'REPLICATED',
                     'options'   :
                       {
      'DISTRIBUTED' : '1',
      'REPLICATED'  : '0'
      }
                     },
  'unique_ids' :   { 're_option' : r'#define\s+LIBMESH_ENABLE_UNIQUE_ID\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   :
                       {
      'TRUE'  : '1',
      'FALSE' : '0'
      }
                     },
  'dtk' :          { 're_option' : r'#define\s+LIBMESH_TRILINOS_HAVE_DTK\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   :
                       {
      'TRUE'  : '1',
      'FALSE' : '0'
      }
                     },
  'vtk' :          { 're_option' : r'#define\s+LIBMESH_HAVE_VTK\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   :
                       {
      'TRUE'  : '1',
      'FALSE' : '0'
      }
                     },
  'tecplot' :      { 're_option' : r'#define\s+LIBMESH_HAVE_TECPLOT_API\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   :
                       {
      'TRUE'  : '1',
      'FALSE' : '0'
      }
                     },
  'petsc_major' :  { 're_option' : r'#define\s+LIBMESH_DETECTED_PETSC_VERSION_MAJOR\s+(\d+)',
                     'default'   : '1'
                   },
  'petsc_minor' :  { 're_option' : r'#define\s+LIBMESH_DETECTED_PETSC_VERSION_MINOR\s+(\d+)',
                     'default'   : '1'
                   },
  'dof_id_bytes' : { 're_option' : r'#define\s+LIBMESH_DOF_ID_BYTES\s+(\d+)',
                     'default'   : '4'
                   },
  'petsc_debug'  : { 're_option' : r'#define\s+LIBMESH_PETSC_USE_DEBUG\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE'  : '1', 'FALSE' : '0'}
                   },
  'curl' :         { 're_option' : r'#define\s+LIBMESH_HAVE_CURL\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
  'tbb' :          { 're_option' : r'#define\s+LIBMESH_HAVE_TBB_API\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
  'superlu' :      { 're_option' : r'#define\s+LIBMESH_PETSC_HAVE_SUPERLU_DIST\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
  'cxx11' :        { 're_option' : r'#define\s+LIBMESH_HAVE_CXX11\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
  'unique_id' :    { 're_option' : r'#define\s+LIBMESH_ENABLE_UNIQUE_ID\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
}


## Run a command and return the output, or ERROR: + output if retcode != 0
def runCommand(cmd, cwd=None):
  # On Windows it is not allowed to close fds while redirecting output
  should_close = platform.system() != "Windows"
  p = Popen([cmd], cwd=cwd, stdout=PIPE,stderr=STDOUT, close_fds=should_close, shell=True)
  output = p.communicate()[0]
  if (p.returncode != 0):
    output = 'ERROR: ' + output
  return output

## print an optionally colorified test result
#
# The test will not be colored if
# 1) options.colored is False,
# 2) the environment variable BITTEN_NOCOLOR is true, or
# 3) the color parameter is False.
def printResult(test_name, result, timing, start, end, options, color=True):
  f_result = ''

  cnt = (TERM_COLS-2) - len(test_name + result)
  color_opts = {'code' : options.code, 'colored' : options.colored}
  if color:
    any_match = False
    # Color leading paths
    m = re.search(r'(.*):(.*)', test_name)
    if m:
      test_name = colorText(m.group(1), 'CYAN', **color_opts) + ':' + m.group(2)
    # Color the Caveats CYAN
    m = re.search(r'(\[.*?\])', result)
    if m:
      any_match = True
      f_result += colorText(m.group(1), 'CYAN', **color_opts) + " "
    # Color Exodiff or CVSdiff tests YELLOW
    m = re.search('(FAILED \((?:EXODIFF|CSVDIFF)\))', result)
    if m:
      any_match = True
      f_result += colorText(m.group(1), 'YELLOW', **color_opts)
    else:
      # Color remaining FAILED tests RED
      m = re.search('(FAILED \(.*\))', result)
      if m:
        any_match = True
        f_result += colorText(m.group(1), 'RED', **color_opts)
    # Color deleted tests RED
    m = re.search('(deleted) (\(.*\))', result)
    if m:
      any_match = True
      f_result += colorText(m.group(1), 'RED', **color_opts) + ' ' + m.group(2)
    # Color long running tests YELLOW
    m = re.search('(RUNNING\.\.\.)', result)
    if m:
      any_match = True
      f_result += colorText(m.group(1), 'YELLOW', **color_opts)
    # Color PBS status CYAN
    m = re.search('((?:LAUNCHED|RUNNING(?!\.)|EXITING|QUEUED))', result)
    if m:
      any_match = True
      f_result += colorText(m.group(1), 'CYAN', **color_opts)
    # Color Passed tests GREEN
    m = re.search('(OK|DRY_RUN)', result)
    if m:
      any_match = True
      f_result += colorText(m.group(1), 'GREEN', **color_opts)

    if not any_match:
      f_result = result

    f_result = test_name + '.'*cnt + ' ' + f_result
  else:
    f_result = test_name + '.'*cnt + ' ' + result

  # Tack on the timing if it exists
  if timing:
    f_result += ' [' + '%0.3f' % float(timing) + 's]'
  if options.debug_harness:
    f_result += ' Start: ' + '%0.3f' % start + ' End: ' + '%0.3f' % end
  return f_result

## Color the error messages if the options permit, also do not color in bitten scripts because
# it messes up the trac output.
# supports weirded html for more advanced coloring schemes. \verbatim<r>,<g>,<y>,<b>\endverbatim All colors are bolded.


def getPlatforms():
  # We'll use uname to figure this out.  platform.uname() is available on all platforms
  #   while os.uname() is not (See bugs.python.org/issue8080).
  # Supported platforms are LINUX, DARWIN, ML, MAVERICKS, YOSEMITE, or ALL
  platforms = set(['ALL'])
  raw_uname = platform.uname()
  if raw_uname[0].upper() == 'DARWIN':
    platforms.add('DARWIN')
    if re.match("12\.", raw_uname[2]):
      platforms.add('ML')
    if re.match("13\.", raw_uname[2]):
      platforms.add("MAVERICKS")
    if re.match("14\.", raw_uname[2]):
      platforms.add("YOSEMITE")
  else:
    platforms.add(raw_uname[0].upper())
  return platforms

def runExecutable(libmesh_dir, location, bin, args):
  # Installed location of libmesh executable
  libmesh_installed   = libmesh_dir + '/' + location + '/' + bin

  # Uninstalled location of libmesh executable
  libmesh_uninstalled = libmesh_dir + '/' + bin

  # Uninstalled location of libmesh executable
  libmesh_uninstalled2 = libmesh_dir + '/contrib/bin/' + bin

  # The eventual variable we will use to refer to libmesh's executable
  libmesh_exe = ''

  if os.path.exists(libmesh_installed):
    libmesh_exe = libmesh_installed

  elif os.path.exists(libmesh_uninstalled):
    libmesh_exe = libmesh_uninstalled

  elif os.path.exists(libmesh_uninstalled2):
    libmesh_exe = libmesh_uninstalled2

  else:
    print "Error! Could not find '" + bin + "' in any of the usual libmesh's locations!"
    exit(1)

  return runCommand(libmesh_exe + " " + args).rstrip()


def getCompilers(libmesh_dir):
  # Supported compilers are GCC, INTEL or ALL
  compilers = set(['ALL'])

  mpicxx_cmd = runExecutable(libmesh_dir, "bin", "libmesh-config", "--cxx")

  # Account for usage of distcc or ccache
  if "distcc" in mpicxx_cmd or "ccache" in mpicxx_cmd:
    mpicxx_cmd = mpicxx_cmd.split()[-1]

  # If mpi ic on the command, run -show to get the compiler
  if "mpi" in mpicxx_cmd:
    raw_compiler = runCommand(mpicxx_cmd + " -show")
  else:
    raw_compiler = mpicxx_cmd

  if re.match('icpc', raw_compiler) != None:
    compilers.add("INTEL")
  elif re.match('[cg]\+\+', raw_compiler) != None:
    compilers.add("GCC")
  elif re.match('clang\+\+', raw_compiler) != None:
    compilers.add("CLANG")

  return compilers

def getPetscVersion(libmesh_dir):
  major_version = getLibMeshConfigOption(libmesh_dir, 'petsc_major')
  minor_version = getLibMeshConfigOption(libmesh_dir, 'petsc_minor')
  if len(major_version) != 1 or len(minor_version) != 1:
    print "Error determining PETSC version"
    exit(1)

  return major_version.pop() + '.' + minor_version.pop()

# Break down petsc version logic in a new define
# TODO: find a way to eval() logic instead
def checkPetscVersion(checks, test):
  # If any version of petsc works, return true immediately
  if 'ALL' in set(test['petsc_version']):
    return (True, None, None)
  # Iterate through petsc versions in test[PETSC_VERSION] and match it against check[PETSC_VERSION]
  for petsc_version in test['petsc_version']:
    logic, version = re.search(r'(.*?)(\d\S+)', petsc_version).groups()
    # Exact match
    if logic == '' or logic == '=':
      if version == checks['petsc_version']:
        return (True, None, version)
      else:
        return (False, '!=', version)
    # Logical match
    if logic == '>' and checks['petsc_version'][0:3] > version[0:3]:
      return (True, None, version)
    elif logic == '>=' and checks['petsc_version'][0:3] >= version[0:3]:
      return (True, None, version)
    elif logic == '<' and checks['petsc_version'][0:3] < version[0:3]:
      return (True, None, version)
    elif logic == '<=' and checks['petsc_version'][0:3] <= version[0:3]:
      return (True, None, version)
  return (False, logic, version)

def getIfAsioExists(moose_dir):
  option_set = set(['ALL'])
  if os.path.exists(moose_dir+"/framework/contrib/asio/include/asio.hpp"):
    option_set.add('TRUE')
  else:
    option_set.add('FALSE')
  return option_set

def getLibMeshConfigOption(libmesh_dir, option):
  # Some tests work differently with parallel mesh enabled
  # We need to detect this condition
  option_set = set(['ALL'])

  filenames = [
    libmesh_dir + '/include/base/libmesh_config.h',   # Old location
    libmesh_dir + '/include/libmesh/libmesh_config.h' # New location
    ];

  success = 0
  for filename in filenames:
    if success == 1:
      break

    try:
      f = open(filename)
      contents = f.read()
      f.close()

      info = LIBMESH_OPTIONS[option]
      m = re.search(info['re_option'], contents)
      if m != None:
        if 'options' in info:
          for value, option in info['options'].iteritems():
            if m.group(1) == option:
              option_set.add(value)
        else:
          option_set.clear()
          option_set.add(m.group(1))
      else:
        option_set.add(info['default'])

      success = 1

    except IOError, e:
      # print "Warning: I/O Error trying to read", filename, ":", e.strerror, "... Will try other locations."
      pass

  if success == 0:
    print "Error! Could not find libmesh_config.h in any of the usual locations!"
    exit(1)

  return option_set

def getSharedOption(libmesh_dir):
  # Some tests may only run properly with shared libraries on/off
  # We need to detect this condition
  shared_option = set(['ALL'])

  result = runExecutable(libmesh_dir, "contrib/bin", "libtool", "--config | grep build_libtool_libs | cut -d'=' -f2")

  if re.search('yes', result) != None:
    shared_option.add('DYNAMIC')
  elif re.search('no', result) != None:
    shared_option.add('STATIC')
  else:
    # Neither no nor yes?  Not possible!
    print "Error! Could not determine whether shared libraries were built."
    exit(1)

  return shared_option

def getInitializedSubmodules(root_dir):
  """
  Gets a list of initialized submodules.
  Input:
    root_dir[str]: path to execute the git command. This should be the root
      directory of the app so that the submodule names are correct
  Return:
    list[str]: List of iniitalized submodule names or an empty list if there was an error.
  """
  output = runCommand("git submodule status", cwd=root_dir)
  if output.startswith("ERROR"):
    return []
  # This ignores submodules that have a '-' at the beginning which means they are not initialized
  return re.findall(r'^[ +]\S+ (\S+)', output, flags=re.MULTILINE)
