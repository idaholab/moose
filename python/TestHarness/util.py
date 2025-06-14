#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
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
from dataclasses import dataclass
import json
import yaml
import sys
import threading
import typing
from typing import Optional
import time

MOOSE_OPTIONS = {
    'ad_size' : { 're_option' : r'#define\s+MOOSE_AD_MAX_DOFS_PER_ELEM\s+(\d+)',
                           'default'   : '64'
    },

    'libpng' :    { 're_option' : r'#define\s+MOOSE_HAVE_LIBPNG\s+(\d+)',
                    'default'   : 'FALSE',
                    'options'   :
                    { 'TRUE'    : '1',
                      'FALSE'   : '0'
                    }
    },

    'libtorch' :    { 're_option' : r'#define\s+MOOSE_LIBTORCH_ENABLED\s+(\d+)',
                    'default'   : 'FALSE',
                    'options'   :
                    { 'TRUE'    : '1',
                      'FALSE'   : '0'
                    }
    },

    'libtorch_dir' : { 're_option' : r'#define\s+MOOSE_LIBTORCH_DIR\s+(.*)',
                       'default'  : '/framework/contrib/libtorch'},

    'mfem' :    { 're_option' : r'#define\s+MOOSE_MFEM_ENABLED\s+(\d+)',
                    'default'   : 'FALSE',
                    'options'   :
                    { 'TRUE'    : '1',
                      'FALSE'   : '0'
                    }
    },

    'mfem_dir' : { 're_option' : r'#define\s+MOOSE_MFEM_DIR\s+(.*)',
                       'default'  : '/framework/contrib/mfem'}
}


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
  'exodus_major' :  { 're_option' : r'#define\s+LIBMESH_DETECTED_EXODUS_VERSION_MAJOR\s+(\d+)',
                     'default'   : '1'
                   },
  'exodus_minor' :  { 're_option' : r'#define\s+LIBMESH_DETECTED_EXODUS_VERSION_MINOR\s+(\d+)',
                     'default'   : '1'
                   },
  'vtk_major' :  { 're_option' : r'#define\s+LIBMESH_DETECTED_VTK_VERSION_MAJOR\s+(\d+)',
                   'default'   : '1'
                 },
  'vtk_minor' :  { 're_option' : r'#define\s+LIBMESH_DETECTED_VTK_VERSION_MINOR\s+(\d+)',
                   'default'   : '1'
                 },
  'vtk_subminor' :  { 're_option' : r'#define\s+LIBMESH_DETECTED_VTK_VERSION_SUBMINOR\s+(\d+)',
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
  'mumps' :        { 're_option' : r'#define\s+LIBMESH_PETSC_HAVE_MUMPS\s+(\d+)',
                     'default'   : 'FALSE',
                     'options'   : {'TRUE' : '1', 'FALSE' : '0'}
                   },
  'strumpack' :        { 're_option' : r'#define\s+LIBMESH_PETSC_HAVE_STRUMPACK\s+(\d+)',
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

LIBTORCH_OPTIONS = {
      'libtorch_major' :  { 're_option' : r'#define\s+TORCH_VERSION_MAJOR\s+(\d+)',
                   'default'   : '1'
                 },
      'libtorch_minor' :  { 're_option' : r'#define\s+TORCH_VERSION_MINOR\s+(\d+)',
                   'default'   : '10'
                 }
}

## Run a command and return the output, or ERROR: + output if retcode != 0
def runCommand(cmd, force_mpi_command=False, **kwargs):
    # On Windows it is not allowed to close fds while redirecting output
    should_close = platform.system() != "Windows"
    if force_mpi_command:
        mpi_command = os.environ.get('MOOSE_MPI_COMMAND')
        if mpi_command is not None:
            cmd = f'{mpi_command} -n 1 {cmd}'
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                       close_fds=should_close, shell=True, text=True, **kwargs)
    output = p.stdout
    if (p.returncode != 0):
        output = 'ERROR: ' + output
    return output

@dataclass(kw_only=True)
class FormatResultEntry:
    """
    Structure for a formatted result to be printed in formatResult()
    """
    # Name of the entry
    name: str
    # Timing for the entry
    timing: float = None
    # JointStatus object for the entry, optional
    joint_status: str = None
    # Detailed status message for the entry, optional
    status_message: str = None
    # Caveats for the entry, optional
    caveats: Optional[list[str]] = None
    # Coloring for the caveats, optional
    caveat_color: Optional[str] = None

def formatResult(entry: FormatResultEntry, options, timing: Optional[bool] = None) -> str:
    """
    Helper for prenting a one-line result for something.

    The entry will not be colored if options.colored = false.
    """
    # Support only one instance of a format identifier, but obey the order
    term_format = [str(v) for v in list(OrderedDict.fromkeys(list(options.term_format)))]
    # container for every printable item (message and color)
    result = dict.fromkeys(term_format, (None, None))

    # Helper for adding a formatted entry
    def add(key: str, message: str, color: str = None) -> None:
        assert key in result
        if message:
            if key.isupper():
                message = message.upper()
            result[key] = (message, color)

    # Populate formatted for those we support, with requested items
    # specified by the user
    caveat_key, justification_key = None, None # filled separately
    for key in term_format:
        key_lower, message, color = key.lower(), None, None

        # Store caveat for later
        if key_lower == 'c':
            caveat_key = key
        # Store justification for later
        elif key_lower == 'j':
            justification_key = key
        # Pre status (not the message, the status type)
        elif key_lower == 'p' and entry.joint_status is not None:
            message, color = entry.joint_status.status.rjust(8, ' '), entry.joint_status.color
        # Status message; only print if the pre status above is not in the result
        # or is in the result and the message above is not the same as this one
        elif key_lower == 's' and entry.status_message is not None and \
            ('p' not in term_format or entry.joint_status is not None and \
             entry.status_message != entry.joint_status.status):
            message = entry.status_message
            color = entry.joint_status.color if entry.joint_status else None
        # Name
        elif key_lower == 'n':
            message = entry.name
        # Time; adjust the precision of time, so we can justify the length.
        # The higher the seconds, the lower the decimal point, ie:
        # [0.000s] - [100.0s]. Max: [99999s]
        elif key_lower == 't' and entry.timing is not None and (options.timing or timing):
            actual = float(entry.timing)
            int_len = len(str(int(actual)))
            precision = min(3, max(0,(4-int_len)))
            message = '[' + '{0: <6}'.format('%0.*fs' % (precision, actual)) + ']'

        add(key, message, color)

    # Helper for the length of the result string so far, removing any color
    def len_result() -> int:
        strip = lambda v: re.sub(r'\033\[\d+m', '', v)
        return len(' '.join([strip(message) for message, _ in result.values() if message]))

    # Decorate Caveats
    if entry.caveats and caveat_key is not None:
        caveats = ','.join(entry.caveats)
        f_caveats = '[' + caveats + ']' # +1 space created later by join
        character_count = len_result() + len(f_caveats) + 1

        # If caveats are the last items the user wants printed, or -e (extra_info) is
        # called, allow caveats to consume available character count beyond options.term_cols.
        # Else, we trim caveats:
        if term_format[-1] != 'c' and not options.extra_info and character_count > options.term_cols:
            over_by_amount = character_count - options.term_cols
            f_caveats = '[' + caveats[:len(caveats) - (over_by_amount + 3)] + '...]'

        add(caveat_key, f_caveats, entry.caveat_color)

    # Fill the available space left, with dots
    if justification_key is not None:
        character_count = len_result() + 1 # +1 space created later by join
        j_dot = '.' * max(0, (options.term_cols - character_count))
        add(justification_key, j_dot, 'GREY')

    # Accumulate values
    values = []
    for message, color in result.values():
        if message:
            if color and options.colored:
                message = colorText(message, color)
            values.append(message)
    return ' '.join(values)

def formatJobResult(job, options, status_message: bool = True, timing: Optional[bool] = None,
                    caveats: bool = False) -> str:
    name = job.getTestName()
    joint_status = job.getJointStatus()

    # Determine status message if requested
    if status_message:
        status_message = joint_status.message if joint_status.message else joint_status.status

        # Add caveats with extra info
        if job.isPass() and options.extra_info:
            for check in options._checks.keys():
                if job.specs.isValid(check) and not 'ALL' in job.specs[check]:
                    job.addCaveats(check)
        # Format failed messages
        elif job.isFail():
            status_message = f'FAILED ({status_message})'
    # Otherwise, just use the status itself
    else:
        status_message = joint_status.status

    # Color first directory if appropriate
    if options.color_first_directory and options.colored:
        first_directory = job.specs['first_directory']
        prefix = colorText(first_directory, 'CYAN')
        suffix = name.replace(first_directory, '', 1)
        name = prefix + suffix

    entry = FormatResultEntry(name=name,
                              timing=job.getTiming(),
                              joint_status=job.getJointStatus(),
                              status_message=status_message,
                              caveats=job.getCaveats() if caveats else [],
                              caveat_color=joint_status.color if job.isFail() else 'CYAN')
    return formatResult(entry, options, timing=timing)

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
    else:
        platforms.add(raw_uname[0].upper())
    return platforms

def getMachine():
    machine = set(['ALL'])
    machine.add(platform.machine().upper())
    return machine

def checkLogicVersionSingle(checks, iversion, package):
    logic, version = re.search(r'(.*?)\s*(\d\S+)', iversion).groups()
    if logic == '' or logic == '=':
        if version == checks[package]:
            return True
        else:
            return False

    from operator import lt, gt, le, ge
    ops = {
      '<': lt,
      '>': gt,
      '<=': le,
      '>=': ge,
    }

    if ops[logic]([int(x) for x in checks[package].split(".")], [int(x) for x in version.split(".")]):
        return True
    return False

def checkVersion(checks, test, package):
    # This is a cheap tokenizer that will split apart the logic into logic groups separated by && and ||
    split_versions_and_logic = re.findall(r".*?(?:(?:&&)|(?:\|\|)|(?:\s*$))", test)

    for group in split_versions_and_logic:
        m = re.search(r'\s*([^\d]*[\d.]*)\s*(\S*)', group)
        if m:
            version, logic_op = m.group(1, 2)
            result = checkLogicVersionSingle(checks, version, package)

            if logic_op == '||':
                if result:
                    return True
            elif logic_op == '&&':
                if not result:
                    return False
            else:
                return result

# Break down petsc version logic in a new define
# TODO: find a way to eval() logic instead
def checkPetscVersion(checks, test):
    # If any version of petsc works, return true immediately
    if 'ALL' in set(test['petsc_version']):
        return (True, None)

    version_string = ' '.join(test['petsc_version'])
    return (checkVersion(checks, version_string, 'petsc_version'), version_string)


# Break down slepc version logic in a new define
def checkSlepcVersion(checks, test):
    # User does not require anything
    if len(test['slepc_version']) == 0:
       return (False, None)
    # SLEPc is not installed
    if checks['slepc_version'] == None:
       return (False, None)
    # If any version of SLEPc works, return true immediately
    if 'ALL' in set(test['slepc_version']):
        return (True, None)

    version_string = ' '.join(test['slepc_version'])
    return (checkVersion(checks, version_string, 'slepc_version'), version_string)

# Break down exodus version logic in a new define
def checkExodusVersion(checks, test):
    version_string = ' '.join(test['exodus_version'])

    # If any version of Exodus works, return true immediately
    if 'ALL' in set(test['exodus_version']):
        return (True, version_string)

    # Exodus not installed or version could not be detected (e.g. old libMesh)
    if checks['exodus_version'] == None:
       return (False, version_string)

    return (checkVersion(checks, version_string, 'exodus_version'), version_string)


# Break down VTKversion logic in a new define
def checkVTKVersion(checks, test):
    version_string = ' '.join(test['vtk_version'])

    # If any version of VTK works, return true immediately
    if 'ALL' in set(test['vtk_version']):
        return (True, version_string)

    # VTK not installed or version could not be detected (e.g. old libMesh)
    if checks['vtk_version'] == None:
       return (False, version_string)

    return (checkVersion(checks, version_string, 'vtk_version'), version_string)

# Break down libtorch version logic in a new define
def checkLibtorchVersion(checks, test):
    version_string = ' '.join(test['libtorch_version'])

    # If any version of libtorch works, return true immediately
    if 'ALL' in set(test['libtorch_version']):
        return (True, version_string)

    # libtorch not installed or version could not be detected
    if checks['libtorch_version'] == None:
       return (False, version_string)

    return (checkVersion(checks, version_string, 'libtorch_version'), version_string)

def outputHeader(header, ending=True):
    """
    Returns text for output with a visual separator, i.e.:
    ##############################...
    <header>
    ##############################...
    """
    begin_sep = '#' * 80
    end_sep = f'{begin_sep}\n' if ending else ''
    return f'{begin_sep}\n{header}\n{end_sep}'

def getCapabilities(exe):
    """
    Get capabilities JSON and compare it to the required capabilities
    """
    assert exe
    try:
        cmd = f'{exe} --show-capabilities'
        output = runCommand(cmd, force_mpi_command=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f'ERROR: Failed to parse the application capabilities!')
        print(f'Command ran: {cmd}\n')
        print(outputHeader(f'Failed command output'))
        print(e.stdout)
        sys.exit(1)
    return parseMOOSEJSON(output, '--show-capabilities')

def getCapability(exe, name):
    """
    Get the value of a capability from a MOOSE application
    """
    value = getCapabilities(exe).get(name)
    return None if value is None else value[0]

def getCapabilityOption(supported: dict,
                        name: str,
                        from_version: bool = False,
                        from_type: type = None,
                        to_set: bool = False,
                        no_all: bool = False,
                        to_bool: bool = False,
                        to_none: bool = False):
    """
    Helper for getting the deprecated Tester option given a capability
    """
    entry = supported.get(name)
    if entry is None:
        raise ValueError(f'Missing capability {name}')
    else:
        value = entry[0]

    if from_version and isinstance(value, str):
        assert re.fullmatch(r'[0-9.]+', value)
    if from_type is not None:
        assert isinstance(value, from_type)

    if value and to_bool:
        value = True

    if to_set:
        values = [str(value).upper()]
        if not no_all:
            values.append('ALL')
        return set(sorted(values))
    else:
        assert not no_all
    if to_none and not value:
        return None
    return value

def checkCapabilities(supported: dict, requested: str, certain):
    """
    Get capabilities JSON and compare it to the required capabilities
    """
    import pycapabilities
    [status, message, doc] = pycapabilities.check(requested, supported)
    success = status == pycapabilities.CERTAIN_PASS or (status == pycapabilities.POSSIBLE_PASS and not certain)
    return success, message

def getSharedOption(libmesh_dir):
    # Some tests may only run properly with shared libraries on/off
    # We need to detect this condition
    shared_option = set(['ALL'])

    libtool = os.path.join(libmesh_dir, "contrib", "bin", "libtool")
    f = open(libtool, "r")

    found = False
    for line in f:
        try:
            (key, value) = line.rstrip().split("=", 2)
        except Exception as e:
            continue

        if key == 'build_libtool_libs':
            if value == 'yes':
                shared_option.add('DYNAMIC')
                found = True
                break
            if value == 'no':
                shared_option.add('STATIC')
                found = True
                break

    f.close()

    if not found:
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
    output = str(runCommand("git submodule status", cwd=root_dir))
    if output.startswith("ERROR"):
        return []
    # This ignores submodules that have a '-' at the beginning which means they are not initialized
    return re.findall(r'^[ +]\S+ (\S+)', output, flags=re.MULTILINE)

def checkInstalled(executable, app_name):
    """
    Read resource file and determine if binary was relocated
    """
    option_set = set(['ALL'])
    if executable:
        resource_content = readResourceFile(executable, app_name)
    else:
        resource_content = {}
    option_set.add(resource_content.get('installation_type', 'ALL').upper())
    return option_set

def parseMOOSEJSON(output: str, context: str) -> dict:
    try:
        output = output.split('**START JSON DATA**\n')[1]
        output = output.split('**END JSON DATA**\n')[0]
        return json.loads(output)
    except IndexError:
        raise Exception(f'Failed to find JSON header and footer from {context}')
    except json.decoder.JSONDecodeError:
        raise Exception(f'Failed to parse JSON from {context}')

def readResourceFile(exe, app_name):
    resource_path = os.path.join(os.path.dirname(os.path.abspath(exe)),
                                 f'{app_name}.yaml')
    if os.path.exists(resource_path):
        try:
            with open(resource_path, 'r', encoding='utf-8') as stream:
                return yaml.safe_load(stream)
        except yaml.YAMLError:
            print(f'resource file parse failure: {resource_path}')
            sys.exit(1)
    return {}

def getRegisteredApps(exe, app_name):
    """
    Gets a list of registered applications
    """
    resource_content = readResourceFile(exe, app_name)
    return resource_content.get('registered_apps', [])

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
                print(("Unable to remove file: " + full_path))

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

def trimOutput(output, max_size=None):
    """ Trims the output given some max size """
    if not max_size or len(output) < max_size or not output:
        return output

    first_part = int(max_size*(2.0/3.0))
    second_part = int(max_size*(1.0/3.0))
    trimmed = f'{output[:first_part]}'
    if trimmed[-1] != '\n':
        trimmed += '\n'
    sep = "#" * 80
    trimmed += f'\n{sep}\nOutput trimmed\n{sep}\n{output[-second_part:]}'
    return trimmed

class ScopedTimer:
    """
    Helper class that will print out a message if a certain amount
    of time has passed
    """
    def __init__(self, timeout: typing.Union[int, float], message: str):
        self.timeout = timeout
        self.message = message
        self._stop_event = threading.Event()
        self._printed = False

    def _check_timeout(self):
        if not self._stop_event.wait(self.timeout):
            print(self.message + '...', end='', flush=True)
            self._printed = True

    def __enter__(self):
        self._thread = threading.Thread(target=self._check_timeout)
        self._thread.start()
        self.start_time = time.time()

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._stop_event.set()
        self._thread.join()
        if self._printed:
            elapsed_time = time.time() - self.start_time
            print(f' {elapsed_time:.2f} seconds', flush=True)
