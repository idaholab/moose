#!/usr/bin/env python
import subprocess, getopt, sys

_USAGE = """
coverage_html.py 'application list'

Where 'application list' is a list of directories to include in
the coverage process. The first item MUST be the base directory
(this will be the title page when generating html content and the
rsync SSL directory).

Exp 1:   coverage_html.py bison fox elk moose some/other/path
Exp 2:   coverage_html.py fox

Exp 1    Will cover bison, fox, elk, moose and some other path.
         While making BISON the title page.
Exp 2    Will cover only fox. And make FOX the title page.
"""

def buildFilter(directory_list):
# build the coverage 'generator' command:
  coverage_cmd = [ 'lcov',
                   '--base-directory', directory_list[0].lower(),
                   '--directory', directory_list[0].lower() + '/src',
                   '--capture',
                   '--ignore-errors', 'gcov,source',
                   '--output-file', 'raw.info'
                   ]
# build the lcov filter command:
  coverage_filter_cmd = [ 'lcov' ]
  for single_filter in directory_list:
    coverage_filter_cmd.extend([ '-e', 'raw.info', '/*' + single_filter.lower() + '/src*', '-e', 'raw.info', '/*' + single_filter.lower() + '/include*' ])
  coverage_filter_cmd.extend([ '-o', 'extracted.info' ])
# build the genhtml command (builds a coverage directory containing the HTML):
  genhtml_cmd = [ 'genhtml', 'extracted.info', '-t', str(directory_list[0].upper()) + " Test Coverage", '--num-spaces', '2', '--legend', '--no-branch-coverage', '-o', 'coverage' ]
# build the rsync command (moves the coverage directory to HPCSC's ssl web server):
  rsync_cmd = [ 'rsync', '-ro', 'coverage', 'hpcsc:/srv/www/ssl/' + str(directory_list[0].upper()) ]

# Actually do stuff:
# run lcov:
  runCMD(coverage_cmd, True)
  runCMD(coverage_filter_cmd, True)
# run genhtml:
  runCMD(genhtml_cmd, True)
# move coverage into place
  runCMD(rsync_cmd)

def printUsage(message):
  sys.stderr.write(_USAGE)
  if message:
    sys.exit('\nFATAL ERROR: ' + message)
  else:
    sys.exit(1)

def process_args():
  try:
    placeholder, opts = getopt.getopt(sys.argv[1:], '', ['help'])
  except getopt.GetoptError:
    printUsage('Invalid arguments.')
  if not opts:
    printUsage('No options specified')
  try:
    if (opts[0] == ''):
      printUsage('Invalid arguments.')
  except:
    printUsage('Invalid arguments.')
  return opts

def runCMD(cmd_opts, quiet=False):
  a_proc = subprocess.Popen(cmd_opts, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  retstr = a_proc.communicate()
  if not a_proc.poll() == 0:
    print 'Error:', retstr[1]
    sys.exit(1)
  else:
    if not quiet:
      print retstr[0]
    return retstr[0]

if __name__ == '__main__':
  buildFilter(process_args())
