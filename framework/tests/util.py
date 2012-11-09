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
    f_result = test_name + '.'*cnt + ' ' + colorify(result, options)
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
def colorify(str, options, html=False):
  # ANSI color codes for colored terminal output
  RESET  = '\033[0m'
  BOLD   = '\033[1m'
  RED    = '\033[31m'
  GREEN  = '\033[32m'
  CYAN   = '\033[36m'
  YELLOW = '\033[33m'

  if options.colored and not (os.environ.has_key('BITTEN_NOCOLOR') and os.environ['BITTEN_NOCOLOR'] == 'true'):
    if html:
      str = str.replace('<r>', BOLD+RED)
      str = str.replace('<g>', BOLD+GREEN)
      str = str.replace('<y>', BOLD+YELLOW)
      str = str.replace('<b>', BOLD)
      str = re.sub(r'</[rgyb]>', RESET, str)
    else:
      str = str.replace('OK', BOLD+GREEN+'OK'+RESET)
      str = re.sub(r'(\[.*?\])', BOLD+CYAN+'\\1'+RESET, str)
      str = str.replace('skipped', BOLD+'skipped'+RESET)
      str = str.replace('deleted', BOLD+RED+'deleted'+RESET)
      if str.find('FAILED (EXODIFF)') != -1:
        str = str.replace('FAILED (EXODIFF)', BOLD+YELLOW+'FAILED (EXODIFF)'+RESET)
      elif str.find('FAILED (CSVDIFF') != -1:
        str = str.replace('FAILED (CSVDIFF)', BOLD+YELLOW+'FAILED (CSVDIFF)'+RESET)
      else:
        str = re.sub(r'(FAILED \([A-Za-z ]*\))', BOLD+RED+'\\1'+RESET, str)
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
  f = open(libmesh_dir + '/Make.common')
  for line in f.readlines():
    if line.find('GXX-VERSION') != -1:
      m = re.search(r'=\s*(\S+)', line)
      if m != None:
        raw_compiler = m.group(1)
        if re.search('intel', raw_compiler, re.I) != None:
          compilers.add("INTEL")
        elif re.search('gcc', raw_compiler, re.I) != None:
          compilers.add("GCC")
        elif re.search('clang', raw_compiler, re.I) != None:
          compilers.add("CLANG")
        break
  f.close()
  return compilers

def getPetscVersion(libmesh_dir):
  # We'll use the petsc-major string from $LIBMESH_DIR/Make.common
  # to figure this out
  # Supported versions are 2, 3
  f = open(libmesh_dir + '/Make.common')
  for line in f.readlines():
    if line.find('petsc-version') != -1:
      m = re.search(r'=\s*(\S+)', line)
      if m != None:
        raw_version = m.group(1)
        petsc_version = raw_version
        break
  f.close()
  return petsc_version

def getParmeshOption(libmesh_dir):
  # Some tests work differently with parallel mesh enabled
  # We need to detect this condition
  parmesh = set()
  f = open(libmesh_dir + '/include/base/libmesh_config.h')
  for line in f.readlines():
    m = re.search(r'#define\s+LIBMESH_ENABLE_PARMESH\s+(\d+)', line)
    if m != None:
      if m.group(1) == '1':
        parmesh.add('PARALLEL')
      else:
        parmesh.add('SERIAL')
      break
  # If we didn't find the #define indicated by having no entries in our set, then parmesh is off
  if not len(parmesh):
    parmesh.add('SERIAL')
  parmesh.add('ALL')
  f.close()
  return parmesh

def getSharedOption(libmesh_dir):
  # Some tests may only run properly with shared libraries on/off
  # We need to detect this condition
  shared_option = set()
  shared_option.add('ALL')
  f = open(libmesh_dir + '/Make.common')
  for line in f.readlines():
    if line.find('enable-shared') != -1:
      m = re.search(r'=\s*(\S+)', line)
      if m != None:
        if m.group(1) == 'yes':
          shared_option.add('DYNAMIC')
        else:
          shared_option.add('STATIC')
        break
  f.close()
  return shared_option
