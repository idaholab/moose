#!/usr/bin/env python
import os, sys, argparse, string, subprocess, time
from argparse import RawTextHelpFormatter

def buildCMD(options):
  tmp_cmd = []
  # Generate Mode
  if options.mode == 'generate':
    # Build lcov base command
    tmp_cmd.append([options.lcov_command[0],
                      '--base-directory', options.application[0],
                      '--directory', options.application[0] + '/src',
                      '--capture',
                      '--ignore-errors', 'gcov,source',
                      '--output-file', 'raw.info'
                     ])
    # Build lcov filter command
    for single_filter in options.application:
      options.lcov_command.extend([ '-e', 'raw.info', '/*' + single_filter + '/src*', '-e', 'raw.info', '/*' + single_filter + '/include*' ])
    options.lcov_command.extend([ '-o', options.outfile ])
    tmp_cmd.append(options.lcov_command)
    tmp_cmd.append(['rm', '-f', 'raw.info'])
    # Build genhtml command if --generate-html was used
    if options.generate_html:
      options.genhtml_command.extend([options.outfile, '-t', options.title + ' Test Coverage', '--num-spaces', '2', '--legend', '--no-branch-coverage', '-o', options.html_location])
      tmp_cmd.append(options.genhtml_command)

  # Combine Mode
  if options.mode == 'combine':
    # Build lcov tracefile command
    for single_tracefile in options.add_tracefile:
      options.lcov_command.extend(['--add-tracefile', single_tracefile])
    options.lcov_command.extend(['-o', options.outfile ])
    tmp_cmd.append(options.lcov_command)
    # Build genhtml command if --generate-html was used
    if options.generate_html:
      options.genhtml_command.extend([options.outfile, '-t', options.title + ' Test Coverage', '--num-spaces', '2', '--legend', '--no-branch-coverage', '-o', options.html_location])
      tmp_cmd.append(options.genhtml_command)

  # Sync Mode
  if options.mode == 'sync':
    options.rsync_command.extend([ '-ro', options.html_location, options.sync_location ])
    tmp_cmd.append(options.rsync_command)

  # Run all built commands. This is where the magic happens
  for single_command in tmp_cmd:
    if options.debug:
      print '\n\nDry Run:', str(options.mode) + ' Mode\n', ' '.join(single_command)
    else:
      runCMD(single_command)

  # Run post-process stuff
  postProcess(options)

def postProcess(options):
  # Add exact time at the bottom of generated HTML coverage
  if options.generate_html and options.addtime:
    addBetterDate(options)

  # Clean up
  if options.cleanup and options.mode is not 'sync':
    cleanUp(options)

def cleanUp(options):
  if options.mode == 'combine':
    for tracefile in options.add_tracefile:
      if options.debug:
        print '\n\nDry Run: deleting file', tracefile
      else:
        try:
          os.remove(tracefile)
        except:
          pass
    if options.debug:
      print '\n\nDry Run: deleting file', options.outfile
    else:
      try:
        os.remove(options.outfile)
      except:
        pass
  if options.mode == 'generate':
    if options.debug:
      print '\n\nDry Run: deleting file', options.outfile
    else:
      try:
        os.remove(options.outfile)
      except:
        pass

def addBetterDate(options):
  if options.debug:
    print '\n\nDry Run: appending timestamp to generated HTML content\n', "echo '" + str(time.ctime(time.time())) + "'", '>>', str(options.html_location) + '/index.html'
  else:
    current_time = time.ctime(time.time())
    output_file = open(options.html_location + '/index.html', 'a')
    output_file.write(current_time)
    output_file.close()

def runCMD(cmd_opts):
  a_proc = subprocess.Popen(cmd_opts, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  retstr = a_proc.communicate()
  if not a_proc.poll() == 0:
    print 'Error:', retstr[1]
    sys.exit(1)
  else:
    return retstr[0]

def exit_error(error_list):
  print '\n\tThere were errors running build_coverage:\n'
  for sgl_error in error_list:
    print sgl_error + '\n'
  sys.exit(1)

def _find(myfile, matchFunc=os.path.isfile):
    paths = string.split(os.getenv('PATH'), os.pathsep)
    for eachpth in paths:
        candidate = os.path.join(eachpth, myfile)
        if matchFunc(candidate):
            return candidate
def find(myfile):
    return _find(myfile)

def _verifyOptions(options):
  error_list = []
  # TODO
  # print ignored commands as warnings?
  # warn_list = []

  # Discover and set the necessary binaries we need
  # find('app_name') returns None if not found
  options.lcov_command = [ find('lcov') ]
  options.genhtml_command = [ find('genhtml') ]
  options.rsync_command = [ find('rsync') ]

  # gererate and combine mode parsing options
  if options.mode == 'generate' or options.mode == 'combine':
    if options.generate_html and options.title is None:
      error_list.append('when generating HTML content, you must specify a title page with --title')
    if options.generate_html and options.html_location is None:
      error_list.append('when generating HTML content, you must specify a save location with --html-location')
    if options.html_location is not None and os.path.exists(options.html_location):
      if options.overwrite is False:
        error_list.append('html location specified already exists. Exiting for safty measures...')
    if options.outfile is None:
      error_list.append('you must specifiy an output file: --outfile')
    if options.outfile is not None and os.path.exists(options.outfile):
      if options.overwrite is False:
        error_list.append('output file specified already exists. Exiting for safty measures...')

  # generate specific parsing options
  if options.mode == 'generate':
    if options.application is None:
      error_list.append('generate mode requires a list of applications to test: --application <list of directories>')

  # combine specific parsing options
  if options.mode == 'combine':
    if options.add_tracefile is None:
      error_list.append('combine mode requires a list of tracefiles to combine: --add-tracefile <list of tracefiles>')

  # sync mode parsing options
  if options.mode == 'sync':
    if options.sync_location is None:
      error_list.append('sync mode requires --sync-location to be set')
    if options.html_location is None:
      error_list.append('sync mode requires --html-location to be set')
    if options.generate_html:
      error_list.append('sync mode does not permit generating HTML content')

  # Did we find all the binaries we need?
  if options.lcov_command[0] == None:
    error_list.append('lcov command not found.')
  if options.genhtml_command[0] == None:
    error_list.append('genhtml command not found.')
  if options.rsync_command[0] == None:
    error_list.append('rsync command not found.')

  # If --dryrun lets print some junk and return
  if options.debug:
    for item in error_list:
      print '\n', item, '\n'
    return options

  # Runs if our list of errors is greater then zero
  if len(error_list) > 0:
    exit_error(error_list)

  # everything appears correct
  return options

def _parseARGs(args=None):
  parser = argparse.ArgumentParser(description='Build code coverage with the option of combining and or transferring HTML \ngenerated content to a specified location (using rsync)', formatter_class=RawTextHelpFormatter)
  parser.add_argument('--generate-html', dest='generate_html', action='store_const', const=True, default=False, help='Generate HTML output. Requires --html-location\n ')
  parser.add_argument('--addtime', dest='addtime', action='store_const', const=True, default=False, help='Add timestamp to code coverage index page\n ')
  parser.add_argument('--overwrite', dest='overwrite', action='store_const', const=True, default=False, help='Ignore files already present\n ')
  parser.add_argument('--cleanup', dest='cleanup', action='store_const', const=True, default=False, help='Delete tracefiles after generating HTML content\n ')
  parser.add_argument('--dryrun', dest='debug', action='store_const', const=True, default=False, help='Do nothing except print what would happen\n ')
  parser.add_argument('--title', help='Title name of code coverage generated HTML \noutput\n ')
  parser.add_argument('--outfile', help='Output tracefile\n ')
  parser.add_argument('--mode', choices=['generate','combine','sync'], default='generate', help='Choose an operational mode:\n\nGENERATE: Generate code coverage for each\nspecified --application (Default)\n\nCOMBINE: Combines tracefiles generated by \ndifferent runs of generate mode\n\nSYNC: Optionally rsync the data to a specified\nlocation. Requires --html-location and\n--sync-location\n ')
  parser.add_argument('--application', metavar='directories', nargs='+', help='A list of Application/s to cover (path to \ndirectories). Used in conjunction with\ngenerate mode.\n ')
  parser.add_argument('--add-tracefile', metavar='tracefiles', nargs='+', help='A list of tracefiles to use in conjunction\nwith combine mode.\n ')
  parser.add_argument('--html-location', help='Location of HTML generated content. Used in\nconjunction with --generate-html or --sync-location\n ')
  parser.add_argument('--sync-location', help='location to rsync the data to:\nuserid@server:/some/location\n ')
  options = parser.parse_args(args)
  return _verifyOptions(options)

if __name__ == '__main__':
  options = _parseARGs()
  buildCMD(options)
