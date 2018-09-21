#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import platform, os, re
import subprocess
from mooseutils import colorText
from collections import OrderedDict
import json

TERM_COLS = int(os.getenv('MOOSE_TERM_COLS', '110'))
TERM_FORMAT = os.getenv('MOOSE_TERM_FORMAT', 'njcst')

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
  'boost' :        { 're_option' : r'#define\s+LIBMESH_HAVE_EXTERNAL_BOOST\s+(\d+)',
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
  'petsc_subminor' :  { 're_option' : r'#define\s+LIBMESH_DETECTED_PETSC_VERSION_SUBMINOR\s+(\d+)',
                     'default'   : '1'
                   },
  'petsc_version_release' :  { 're_option' : r'#define\s+LIBMESH_DETECTED_PETSC_VERSION_RELEASE\s+(\d+)',
                     'default'   : 'TRUE',
                     'options'   : {'TRUE'  : '1', 'FALSE' : '0'}
                   },
  'slepc_major' :  { 're_option' : r'#define\s+LIBMESH_DETECTED_SLEPC_VERSION_MAJOR\s+(\d+)',
                     'default'   : '1'
                   },
  'slepc_minor' :  { 're_option' : r'#define\s+LIBMESH_DETECTED_SLEPC_VERSION_MINOR\s+(\d+)',
                     'default'   : '1'
                   },
  'slepc_subminor' :  { 're_option' : r'#define\s+LIBMESH_DETECTED_SLEPC_VERSION_SUBMINOR\s+(\d+)',
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
  'threads' :      { 're_option' : r'#define\s+LIBMESH_USING_THREADS\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
  'tbb' :          { 're_option' : r'#define\s+LIBMESH_HAVE_TBB_API\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
  'openmp' :       { 're_option' : r'#define\s+LIBMESH_HAVE_OPENMP\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
  'superlu' :      { 're_option' : r'#define\s+LIBMESH_PETSC_HAVE_SUPERLU_DIST\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
  'parmetis' :      { 're_option' : r'#define\s+LIBMESH_(?:PETSC_){0,1}HAVE_PARMETIS\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
  'chaco' :      { 're_option' : r'#define\s+LIBMESH_PETSC_HAVE_CHACO\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
  'party' :      { 're_option' : r'#define\s+LIBMESH_PETSC_HAVE_PARTY\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
  'ptscotch' :      { 're_option' : r'#define\s+LIBMESH_PETSC_HAVE_PTSCOTCH\s+(\d+)',
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
  'fparser_jit' :  { 're_option' : r'#define\s+LIBMESH_HAVE_FPARSER_JIT\s+(\d+)',
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


## method to return current character count with given results_dictionary
def resultCharacterCount(results_dict):
    # { formatted_result_key : ( text, color ) }
    printable_items = []
    for result_key, printable in results_dict.iteritems():
        if printable:
            printable_items.append(printable[0])
    return len(' '.join(printable_items))

## convert the incoming message tuple to the same case, as the case of format_key
## store this information to the same cased key in formatted_results dict.
def formatCase(format_key, message, formatted_results):
    if message and format_key.isupper():
        formatted_results[format_key] = (message[0].upper(), message[1])
    elif message:
        formatted_results[format_key] = (message[0], message[1])

def formatStatusMessage(tester, options):
    # PASS and DRY_RUN fall into this catagory
    if tester.isPass():
        result = tester.getStatusMessage()

        if options.extra_info:
            for check in options._checks.keys():
                if tester.specs.isValid(check) and not 'ALL' in tester.specs[check]:
                    tester.addCaveats(check)

    # FAIL, DIFF and DELETED fall into this catagory
    elif tester.isFail() or (tester.isDeleted() and options.extra_info):
        message = tester.getStatusMessage()
        if message == '':
            message = tester.getStatus().status

        result = 'FAILED (%s)' % (message)

    # Some other finished status... skipped, silent, etc
    else:
        result = tester.getStatusMessage()

    # No one set a unique status message? In that case, use the name of
    # the status itself as the status message.
    if not result:
        result = tester.getStatus().status

    return result

## print an optionally colorified test result
#
# The test will not be colored if
# 1) options.colored is False,
# 2) the color parameter is False.
def formatResult(job, options, result='', color=True, **kwargs):
    # Support only one instance of a format identifier, but obey the order
    terminal_format = list(OrderedDict.fromkeys(list(TERM_FORMAT)))
    tester = job.getTester()
    if tester.isNoStatus():
        status = job.getStatus()
    else:
        status = tester.getStatus()
    color_opts = {'code' : options.code, 'colored' : options.colored}

    # container for every printable item
    formatted_results = dict.fromkeys(terminal_format)

    # Populate formatted_results for those we support, with requested items
    # specified by the user. Caveats and justifications are parsed outside of
    # loop as these two items change based on character count consumed by others.
    caveat_index = None
    justification_index = None
    for i, f_key in enumerate(terminal_format):
        # Store the caveat request. We will use this later.
        if str(f_key).lower() == 'c':
            caveat_index = terminal_format[i]

        # Store the justification request. We will use this later.
        if str(f_key).lower() == 'j':
            justification_index = terminal_format[i]

        if str(f_key).lower() == 'p':
            pre_result = ' '*(8-len(status.status)) + status.status
            formatCase(f_key, (pre_result, status.color), formatted_results)

        if str(f_key).lower() == 's':
            if not result:
                result = formatStatusMessage(tester, options)

            # refrain from printing a duplicate pre_result if it will match result
            if 'p' in [x.lower() for x in terminal_format] and result == status.status:
                formatCase(f_key, None, formatted_results)
            else:
                formatCase(f_key, (result, status.color), formatted_results)

        if str(f_key).lower() == 'n':
            formatCase(f_key, (tester.getTestName(), None), formatted_results)

        # Adjust the precision of time, so we can justify the length. The higher the
        # seconds, the lower the decimal point, ie: [0.000s] - [100.0s]. Max: [99999s]
        if str(f_key).lower() == 't' and options.timing:
            actual = float(job.getTiming())
            int_len = len(str(int(actual)))
            precision = min(3, max(0,(4-int_len)))
            f_time = '[' + '{0: <6}'.format('%0.*fs' % (precision, actual)) + ']'
            formatCase(f_key, (f_time, None), formatted_results)

    # Decorate Caveats
    if tester.getCaveats() and caveat_index is not None and 'caveats' in kwargs and kwargs['caveats']:
        caveats = ','.join(tester.getCaveats())
        caveat_color = status.color
        if tester.isPass() or tester.isSkip():
            caveat_color = 'CYAN'

        f_caveats = '[' + caveats + ']'
        # +1 space created later by join
        character_count = resultCharacterCount(formatted_results) + len(f_caveats) + 1

        # If caveats are the last items the user wants printed, or -e (extra_info) is
        # called, allow caveats to consume available character count beyond TERM_COLS.
        # Else, we trim caveats:
        if terminal_format[-1].lower() != 'c' \
           and not options.extra_info \
           and character_count > TERM_COLS:
            over_by_amount = character_count - TERM_COLS
            f_caveats = '[' + caveats[:len(caveats) - (over_by_amount + 3)] + '...]'

        formatCase(caveat_index, (f_caveats, caveat_color), formatted_results)

    # Fill the available space left, with dots
    if justification_index is not None:
        j_dot = None
        # +1 space created later by join
        character_count = resultCharacterCount(formatted_results) + 1
        if character_count < TERM_COLS:
            j_dot = ('.'*max(0, (TERM_COLS - character_count)), 'GREY')
        elif character_count == TERM_COLS:
            j_dot = ('', 'GREY')

        formatCase(justification_index, j_dot, formatted_results)

    # If color, decorate those items which support it
    if color:
        for format_rule, printable in formatted_results.iteritems():
            if printable and (printable[0] and printable[1]):
                formatted_results[format_rule] = (colorText(printable[0], printable[1], **color_opts), printable[1])

            # Do special coloring for first directory
            if format_rule == 'n' and options.color_first_directory:
                formatted_results[format_rule] = (colorText(tester.specs['first_directory'], 'CYAN', **color_opts) +\
                                         formatted_results[format_rule][0].replace(tester.specs['first_directory'], '', 1), 'CYAN') # Strip out first occurence only

    # join printable results in the order in which the user asked
    final_results = ' '.join([formatted_results[x][0] for x in terminal_format if formatted_results[x]])

    # Decorate debuging
    if options.debug_harness:
        final_results += ' Start: ' + '%0.3f' % job.getStartTime() + ' End: ' + '%0.3f' % job.getEndTime()

    return final_results

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
        print("Error! Could not find '" + bin + "' in any of the usual libmesh's locations!")
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

def getLibMeshThreadingModel(libmesh_dir):
    threading_models = set(['ALL'])
    have_threads = 'TRUE' in getLibMeshConfigOption(libmesh_dir, 'threads');
    if have_threads:
        have_tbb = 'TRUE' in getLibMeshConfigOption(libmesh_dir, 'tbb')
        have_openmp = 'TRUE' in getLibMeshConfigOption(libmesh_dir, 'openmp')
        if have_openmp:
            threading_models.add("OPENMP")
        elif have_tbb:
            threading_models.add("TBB")
        else:
            threading_models.add("PTHREADS")
    else:
        threading_models.add("NONE")
    return threading_models

def getPetscVersion(libmesh_dir):
    major_version = getLibMeshConfigOption(libmesh_dir, 'petsc_major')
    minor_version = getLibMeshConfigOption(libmesh_dir, 'petsc_minor')
    subminor_version = getLibMeshConfigOption(libmesh_dir, 'petsc_subminor')
    if len(major_version) != 1 or len(minor_version) != 1:
        print "Error determining PETSC version"
        exit(1)

    return major_version.pop() + '.' + minor_version.pop() + '.' + subminor_version.pop()

def getSlepcVersion(libmesh_dir):
    major_version = getLibMeshConfigOption(libmesh_dir, 'slepc_major')
    minor_version = getLibMeshConfigOption(libmesh_dir, 'slepc_minor')
    subminor_version = getLibMeshConfigOption(libmesh_dir, 'slepc_subminor')
    if len(major_version) != 1 or len(minor_version) != 1 or len(major_version) != 1:
      return None

    return major_version.pop() + '.' + minor_version.pop() + '.' + subminor_version.pop()

def checkLogicVersionSplits(target, splits, logic_and, package):
    status, logic_reason, version = [],[],[]
    for split in splits:
        (lstatus, lreason, lversion) = checkLogicVersionSingle(target, split, package)
        status.append(lstatus)
        logic_reason.append(lreason)
        version.append(lversion)

    if logic_and:
        if status[0] and status[1]:
            return (True, None, version[0])
        else:
            return (False, '&&', version[0]+' '+version[1])
    else:
        if status[0] or status[1]:
            return (True, None, version[0])
        else:
            return (True, '||', version[0]+ ' '+version[1])

    return (False, None, None)

def checkLogicVersionSingle(checks, iversion, package):
    logic, version = re.search(r'(.*?)(\d\S+)', iversion).groups()
    if logic == '' or logic == '=':
        if version == checks[package]:
            return (True, None, version)
        else:
            return (False, '!=', version)

    # Logical match
    if logic == '>' and checks[package][0:5] > version[0:5]:
        return (True, None, version)
    elif logic == '>=' and checks[package][0:5] >= version[0:5]:
        return (True, None, version)
    elif logic == '<' and checks[package][0:5] < version[0:5]:
        return (True, None, version)
    elif logic == '<=' and checks[package][0:5] <= version[0:5]:
        return (True, None, version)

    return (False, logic, version)

def checkVersion(checks, test, package):
    for version in test[package]:
        logic_and = False
        if version.find("&&") != -1:
            logic_and = True
        splits = re.split('[(&&) | (||)]+', version)
        if len(splits) == 1:
            return checkLogicVersionSingle(checks, version, package)
        elif len(splits) == 2:
            return checkLogicVersionSplits(checks, splits, logic_and, package)
        else:
            print "Invalid expression: " + version
            exit(1)

    return (False, None, None)

# Break down petsc version logic in a new define
# TODO: find a way to eval() logic instead
def checkPetscVersion(checks, test):
    # If any version of petsc works, return true immediately
    if 'ALL' in set(test['petsc_version']):
        return (True, None, None)

    return checkVersion(checks, test, 'petsc_version')


# Break down slepc version logic in a new define
def checkSlepcVersion(checks, test):
    # User does not require anything
    if len(test['slepc_version']) == 0:
       return (False, None, None)
    # SLEPc is not installed
    if checks['slepc_version'] == None:
       return (False, None, None)
    # If any version of SLEPc works, return true immediately
    if 'ALL' in set(test['slepc_version']):
        return (True, None, None)

    return checkVersion(checks, test, 'slepc_version')

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
        print("Error! Could not determine whether shared libraries were built.")
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

def addObjectsFromBlock(objs, node, block_name):
    """
    Utility function that iterates over a dictionary and adds keys
    to the executable object name set.
    """
    data = node.get(block_name, {})
    if data: # could be None so we can't just iterate over items
        for name, block in data.iteritems():
            objs.add(name)
            addObjectNames(objs, block)

def addObjectNames(objs, node):
    """
    Add object names that reside in this node.
    """
    if not node:
        return

    addObjectsFromBlock(objs, node, "subblocks")
    addObjectsFromBlock(objs, node, "subblock_types")
    addObjectsFromBlock(objs, node, "types")

    star = node.get("star")
    if star:
        addObjectNames(objs, star)

def getExeJSON(exe):
    """
    Extracts the JSON from the dump
    """
    output = runCommand("%s --json" % exe)
    output = output.split('**START JSON DATA**\n')[1]
    output = output.split('**END JSON DATA**\n')[0]
    return json.loads(output)

def getExeObjects(exe):
    """
    Gets a set of object names that are in the executable JSON dump.
    """
    data = getExeJSON(exe)
    obj_names = set()
    addObjectsFromBlock(obj_names, data, "blocks")
    return obj_names

def getExeRegisteredApps(exe):
    """
    Gets a list of registered applications
    """
    data = getExeJSON(exe)
    return data.get('global', {}).get('registered_apps', [])

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
                print("Unable to remove file: " + full_path)

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

# Check if test has any redirected output, and if its ready to be read
def checkOutputReady(tester, options):
    checked_files = []
    for redirected_file in tester.getRedirectedOutputFiles(options):
        file_path = os.path.join(tester.getTestDir(), redirected_file)
        if os.access(file_path, os.R_OK):
            checked_files.append(file_path)
    return checked_files

# return concatenated output from tests with redirected output
def getOutputFromFiles(tester, options):
    file_output = ''
    output_files = checkOutputReady(tester, options)
    for file_path in output_files:
        with open(file_path, 'r') as f:
            file_output += "#"*80 + "\nOutput from " + file_path \
                           + "\n" + "#"*80 + "\n" + readOutput(f, None, options, max_size=tester.specs['max_buffer_size'])
    return file_output

# This function reads output from the file (i.e. the test output)
# but trims it down to the specified size.  It'll save the first two thirds
# of the requested size and the last third trimming from the middle
def readOutput(f, e, options, max_size=None):
    if max_size is None:
        max_size = 100000

    first_part = int(max_size*(2.0/3.0))
    second_part = int(max_size*(1.0/3.0))
    output = ''

    f.seek(0)
    if e:
        e.seek(0)
    if options.no_trimmed_output or max_size == -1:
        output += f.read()
        if e:
            output += e.read()

    else:
        output = f.read(first_part)     # Limit the output to 1MB
        if len(output) == first_part:   # This means we didn't read the whole file yet
            output += "\n" + "#"*80 + "\n\nOutput trimmed\n\n" + "#"*80 + "\n"
            f.seek(-second_part, 2)       # Skip the middle part of the file

            if (f.tell() <= first_part):  # Don't re-read some of what you've already read
                f.seek(first_part+1, 0)

        output += f.read()          # Now read the rest
        if e:
            output += e.read()      # Do not trim errors
    return output
