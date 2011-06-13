import os, re
from subprocess import *

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
def printResult(test_name, result, options, color=True):
  cnt = 79 - len(test_name + result)
  if color:
    return test_name + '.'*cnt + ' ' + colorify(result, options)
  else:
    return test_name + '.'*cnt + ' ' + result

## Color the error messages if the options permit, also do not color in bitten scripts because
# it messes up the trac output.
# supports weirded html for more advanced coloring schemes. <r>,<g>,<y>,<b> All colors are bolded.
def colorify(str, options, html=False):
  # ANSI color codes for colored terminal output
  RESET  = '\033[0m'
  BOLD   = '\033[1m'
  RED    = '\033[31m'
  GREEN  = '\033[32m'
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
      str = str.replace('skipped', BOLD+'skipped'+RESET)
      str = re.sub(r'(FAILED \([A-Za-z ]*\))', BOLD+RED+'\\1'+RESET, str)
      #str = str.replace('FAILED', BOLD+RED+'FAILED'+RESET)
      #str = str.replace('FAILED (DIFF)', BOLD+YELLOW+'FAILED (DIFF)'+RESET)
  elif html:
    str = re.sub(r'</?[rgyb]>', '', str)    # strip all "html" tags

  return str
