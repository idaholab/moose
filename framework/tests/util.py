import os, re
from subprocess import *
from time import strftime, gmtime, ctime, localtime, asctime

TERM_COLS = 110

## Run a command and return the output, or ERROR: + output if retcode != 0
def runCommand(cmd):
  p = Popen([cmd],stdout=PIPE,stderr=STDOUT, close_fds=True, shell=True)
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
  if color:
    any_match = False

    # Color the Caveats CYAN
    m = re.search(r'(\[.*?\])', result)
    if m:
      any_match = True
      f_result += colorText(m.group(1), options, 'CYAN') + " "
    # Color Exodiff or CVSdiff tests YELLOW
    m = re.search('(FAILED \((?:EXODIFF|CVSDIFF)\))', result)
    if m:
      any_match = True
      f_result += colorText(m.group(1), options, 'YELLOW')
    else:
      # Color remaining FAILED tests RED
      m = re.search('(FAILED \(.*\))', result)
      if m:
        any_match = True
        f_result += colorText(m.group(1), options, 'RED')
    # Color deleted tests RED
    m = re.search('(deleted) (\(.*\))', result)
    if m:
      any_match = True
      f_result += colorText(m.group(1), options, 'RED') + ' ' + m.group(2)
    # Color Passed tests GREEN
    m = re.search('(OK)', result)
    if m:
      any_match = True
      f_result += colorText(m.group(1), options, 'GREEN')

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
# supports weirded html for more advanced coloring schemes. <r>,<g>,<y>,<b> All colors are bolded.

def colorText(str, options, color, html=False):
  # ANSI color codes for colored terminal output
  color_codes = {'RESET':'\033[0m','BOLD':'\033[1m','RED':'\033[31m',
                 'GREEN':'\033[32m' if options.code else '\033[35m',
                 'CYAN':'\033[36m' if options.code else '\033[34m',
                 'YELLOW':'\033[33m'}

  if options.colored and not (os.environ.has_key('BITTEN_NOCOLOR') and os.environ['BITTEN_NOCOLOR'] == 'true'):
    if html:
      str = str.replace('<r>', color_codes['BOLD']+color_codes['RED'])
      str = str.replace('<g>', color_codes['BOLD']+color_codes['GREEN'])
      str = str.replace('<y>', color_codes['BOLD']+color_codes['YELLOW'])
      str = str.replace('<b>', color_codes['BOLD'])
      str = re.sub(r'</[rgyb]>', color_codes['RESET'], str)
    else:
      str = color_codes[color] + str + color_codes['RESET']
  elif html:
    str = re.sub(r'</?[rgyb]>', '', str)    # strip all "html" tags

  return str

def getPlatforms():
  # We'll use uname to figure this out
  # Supported platforms are LINUX, DARWIN, SL, LION or ALL
  platforms = set()
  platforms.add('ALL')
  raw_uname = os.uname()
  if raw_uname[0].upper() == 'DARWIN':
    platforms.add('DARWIN')
    if re.match("10\.", raw_uname[2]):
      platforms.add('SL')
    if re.match("11\.", raw_uname[2]):
      platforms.add("LION")
  else:
    platforms.add(raw_uname[0].upper())
  return platforms

def getCompilers(libmesh_dir):
  # We'll use the GXX-VERSION string from LIBMESH's Make.common
  # to figure this out
  # Supported compilers are GCC, INTEL or ALL
  compilers = set()
  compilers.add('ALL')

  # Get the gxx compiler.  Note that the libmesh-config script
  # can live in different places depending on whether libmesh is
  # "installed" or not.

  # Installed location of libmesh-config script
  libmesh_config_installed   = libmesh_dir + '/bin/libmesh-config'

  # Uninstalled location of libmesh-config script
  libmesh_config_uninstalled = libmesh_dir + '/contrib/bin/libmesh-config'

  # The eventual variable we will use to refer to libmesh's configure script
  libmesh_config = ''

  if os.path.exists(libmesh_config_installed):
    libmesh_config = libmesh_config_installed

  elif os.path.exists(libmesh_config_uninstalled):
    libmesh_config = libmesh_config_uninstalled

  else:
    print "Error! Could not find libmesh's config script in any of the usual locations!"
    exit(1)

  # Pass the --cxx option to the libmesh-config script, and check the result
  command = libmesh_config + ' --cxx'
  p = Popen(command, shell=True, stdout=PIPE)
  mpicxx_cmd = p.communicate()[0].strip()

  p = Popen(mpicxx_cmd + " --show", shell=True, stdout=PIPE)
  raw_compiler = p.communicate()[0]

  if re.search('icpc', raw_compiler) != None:
    compilers.add("INTEL")
  elif re.search('g\+\+', raw_compiler) != None:
    compilers.add("GCC")
  elif re.search('clang\+\+', raw_compiler) != None:
    compilers.add("CLANG")

  return compilers

def getPetscVersion(libmesh_dir):
  # We'll use PETSc's own header file to determine the PETSc version
  # (major.minor).  If necessary in the future we'll also detect subminor...
  #
  # Note: we used to find this info in Make.common, but in the
  # automake version of libmesh, this information is no longer stored
  # in Make.common, but rather in
  #
  # $LIBMESH_DIR/lib/${AC_ARCH}_${METHOD}/pkgconfig/Make.common.${METHOD}
  #
  # where ${AC_ARCH} is an architecture-dependent string determined by
  # libmesh's config.guess.  So we could try to look there, but it's
  # easier and more portable to look in ${PETSC_DIR}.

  # Default to something that doesn't make sense
  petsc_version_major = 'x'
  petsc_version_minor = 'x'

  # Get user's PETSC_DIR from environment.
  petsc_dir = os.environ.get('PETSC_DIR')

  # environ.get returns 'none' if no such environment variable exists.
  if petsc_dir == 'none':
    print "PETSC_DIR not found in environment!  Cannot detect PETSc version!"
    exit(1)

  # FIXME: handle I/O exceptions when opening this file
  f = open(petsc_dir + '/include/petscversion.h')

  # The version lines are (hopefully!) always going to be of the form
  # #define PETSC_VERSION_MAJOR      X
  # where X is some number, so in python, we can split the string and
  # pop the last substring (the version) off the end.
  for line in f.readlines():
    if line.find('#define PETSC_VERSION_MAJOR') != -1:
      petsc_version_major = line.split().pop()

    elif line.find('#define PETSC_VERSION_MINOR') != -1:
      petsc_version_minor = line.split().pop()

    # See if we're done.
    if (petsc_version_major != 'x' and petsc_version_minor != 'x'):
      break

  # Done with the file, so we can close it now
  f.close()

  # If either version was not found, then we can't continue :(
  if petsc_version_major == 'x':
    print("Error: could not determine valid PETSc major version.")
    exit(1)

  if petsc_version_minor == 'x':
    print("Error: could not determine valid PETSc minor version.")
    exit(1)

  petsc_version = petsc_version_major + '.' + petsc_version_minor

  # print "Running tests assuming PETSc version", petsc_version

  return petsc_version



def getParmeshOption(libmesh_dir):
  # Some tests work differently with parallel mesh enabled
  # We need to detect this condition
  parmesh = set()

  # We check libmesh_config.h for LIBMESH_ENABLE_PARMESH
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

      for line in f.readlines():
        m = re.search(r'#define\s+LIBMESH_ENABLE_PARMESH\s+(\d+)', line)
        if m != None:
          if m.group(1) == '1':
            parmesh.add('PARALLEL')
          else:
            parmesh.add('SERIAL')
          break

      f.close()
      success = 1

    except IOError, e:
      # print "Warning: I/O Error trying to read", filename, ":", e.strerror, "... Will try other locations."
      pass


  if success == 0:
    print "Error! Could not find libmesh_config.h in any of the usual locations!"
    exit(1)


  # If we didn't find the #define indicated by having no entries in our set, then parmesh is off
  if not len(parmesh):
    parmesh.add('SERIAL')
  parmesh.add('ALL')

  return parmesh





def getSharedOption(libmesh_dir):
  # Some tests may only run properly with shared libraries on/off
  # We need to detect this condition
  shared_option = set()
  shared_option.add('ALL')

  # MOOSE no longer relies on Make.common being present.  This gives us the
  # potential to work with "uninstalled" libmesh trees, for example.

  # Installed location of libmesh libtool script
  libmesh_libtool_installed   = libmesh_dir + '/contrib/bin/libtool'

  # Uninstalled location of libmesh libtool script
  libmesh_libtool_uninstalled = libmesh_dir + '/libtool'

  # The eventual variable we will use to refer to libmesh's libtool script
  libmesh_libtool = ''

  if os.path.exists(libmesh_libtool_installed):
    libmesh_libtool = libmesh_libtool_installed

  elif os.path.exists(libmesh_libtool_uninstalled):
    libmesh_libtool = libmesh_libtool_uninstalled

  else:
    print "Error! Could not find libmesh's libtool script in any of the usual locations!"
    exit(1)

  # Now run the libtool script (in the shell) to see if shared libraries were built
  command = libmesh_libtool + " --config | grep build_libtool_libs | cut -d'=' -f2"

  # Note: the strip() command removes the trailing newline
  p = Popen(command, shell=True, stdout=PIPE)
  result = p.communicate()[0].strip()

  if re.search('yes', result) != None:
    shared_option.add('DYNAMIC')
  elif re.search('no', result) != None:
    shared_option.add('STATIC')
  else:
    # Neither no nor yes?  Not possible!
    print "Error! Could not determine whether shared libraries were built."
    exit(1)

  return shared_option
