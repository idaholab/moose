import platform, os, re
import subprocess
from mooseutils import colorText
from collections import namedtuple

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
  'slepc' :        { 're_option' : r'#define\s+LIBMESH_HAVE_SLEPC\s+(\d+)',
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
    p = subprocess.Popen([cmd], cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=should_close, shell=True)
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
def printResult(tester, result, timing, start, end, options, color=True):
    f_result = ''
    caveats = ''
    first_directory = tester.specs['first_directory']
    test_name = tester.specs['test_name']
    status = tester.getStatus()

    cnt = (TERM_COLS-2) - len(test_name + result)
    color_opts = {'code' : options.code, 'colored' : options.colored}
    if color:
        if options.color_first_directory:
            test_name = colorText(first_directory, 'CYAN', **color_opts) + test_name.replace(first_directory, '', 1) # Strip out first occurence only
        # Color the Caveats CYAN
        m = re.search(r'(\[.*?\])', result)
        if m:
            caveats = m.group(1)
            f_result = colorText(caveats, 'CYAN', **color_opts)

        # Color test results based on status.
        # Keep any caveats that may have been colored
        if status:
            f_result += colorText(result.replace(caveats, ''), tester.getColor(), **color_opts)

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

        except IOError:
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

def checkOutputForPattern(output, re_pattern):
    """
    Returns boolean of pattern match
    """
    if re.search(re_pattern, output, re.MULTILINE | re.DOTALL) == None:
        return False
    else:
        return True

def checkOutputForLiteral(output, literal):
    """
    Returns boolean of literal match
    """
    if output.find(literal) == -1:
        return False
    else:
        return True

def deleteFilesAndFolders(test_dir, paths, delete_folders=True):
    """
    Delete specified files

    test_dir:       The base test directory
    paths:          A list contianing files to delete
    delete_folders: Attempt to delete any folders created
    """
    for file in paths:
        full_path = os.path.join(test_dir, file)
        if os.path.exists(full_path):
            try:
                os.remove(full_path)
            except:
                print "Unable to remove file: " + full_path

    # Now try to delete directories that might have been created
    if delete_folders:
        for file in paths:
            path = os.path.dirname(file)
            while path != '':
                (path, tail) = os.path.split(path)
                try:
                    os.rmdir(os.path.join(test_dir, path, tail))
                except:
                    # There could definitely be problems with removing the directory
                    # because it might be non-empty due to checkpoint files or other
                    # files being created on different operating systems. We just
                    # don't care for the most part and we don't want to error out.
                    # As long as our test boxes clean before each test, we'll notice
                    # the case where these files aren't being generated for a
                    # particular run.
                    #
                    # TL;DR; Just pass...
                    pass

# See http://code.activestate.com/recipes/576570-dependency-resolver/
class DependencyResolver:
    def __init__(self):
        self.dependency_dict = {}

    def insertDependency(self, key, values):
        self.dependency_dict[key] = values

    def getSortedValuesSets(self):
        d = dict((k, set(self.dependency_dict[k])) for k in self.dependency_dict)
        r = []
        while d:
            # values not in keys (items without dep)
            t = set(i for v in d.values() for i in v) - set(d.keys())
            # and keys without value (items without dep)
            t.update(k for k, v in d.items() if not v)

            if len(t) == 0 and len(d) > 0:
              raise Exception("Cyclic or Invalid Dependency Detected!")

            # can be done right away
            r.append(t)
            # and cleaned up
            d = dict(((k, v-t) for k, v in d.items() if v))
        return r


class TestStatus(object):
    """
    Class for handling test statuses
    """

    ###### bucket status discriptions
    ## The following is a list of statuses possible in the TestHarness
    ##
    ## PASS    =  Passing tests 'OK'
    ## FAIL    =  Failing tests
    ## DIFF    =  Failing tests due to Exodiff, CSVDiff
    ## PBS     =  Any statuses belonging to messages generated by PBS
    ## PENDING =  A pending status applied by the TestHarness (RUNNING...)
    ## DELETED =  A skipped test hidden from reporting. Under normal circumstances, this sort of test
    ##            is placed in the SILENT bucket. It is only placed in the DELETED bucket (and therfor
    ##            printed to stdout) when the user has specifically asked for more information while
    ##            running tests (-e)
    ## SKIP    =  Any test reported as skipped
    ## SILENT  =  Any test reported as skipped and should not alert the user (deleted, tests not
    ##            matching '--re=' options, etc)
    ######

    test_status    = namedtuple('test_status', 'status color')
    bucket_success = test_status(status='PASS', color='GREEN')
    bucket_fail    = test_status(status='FAIL', color='RED')
    bucket_deleted = test_status(status='DELETED', color='RED')
    bucket_diff    = test_status(status='DIFF', color='YELLOW')
    bucket_pbs     = test_status(status='PBS', color='CYAN')
    bucket_pending = test_status(status='PENDING', color='CYAN')
    bucket_skip    = test_status(status='SKIP', color='RESET')
    bucket_silent  = test_status(status='SILENT', color='RESET')

    # Initialize the class with a pending status
    # TODO: don't do this? Initialize instead with None type? If we do
    # and forget to set a status, getStatus will fail with None type errors
    def __init__(self, status_message='initialized', status=bucket_pending):
        self.__status_message = status_message
        self.__status = status

    def setStatus(self, status_message, status_bucket):
        """
        Set bucket status
          setStatus("reason", TestStatus.bucket_tuple)
        """
        self.__status_message = status_message
        self.__status = status_bucket

    def getStatus(self):
        """
        Return status bucket namedtuple
        """
        return self.__status

    def getStatusMessage(self):
        """
        Return status message string
        """
        return self.__status_message

    def getColor(self):
        """
        Return enumerated color string
        """
        return self.__status.color

    def didPass(self):
        """
        Return boolean passing status (True if passed)
        """
        return self.getStatus() == self.bucket_success

    def didFail(self):
        """
        Return boolean failing status (True if failed)
        """
        status = self.getStatus()
        return status == self.bucket_fail or status == self.bucket_diff

    def getRunnable(self):
        """
        Return boolean whether the test should be allowed to run or not
        """
        status = self.getStatus()
        return not (status == self.bucket_deleted or status == self.bucket_skip or status == self.bucket_silent)
