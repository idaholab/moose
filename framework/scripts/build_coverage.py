#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, sys, argparse, string, subprocess, time, re
from argparse import RawTextHelpFormatter

def buildCMD(options):
    tmp_cmd = []
    # Store any additional directories supplied in a temp list
    if options.mode != 'combine' and options.mode != 'sync':
        tmp_additional_directories = str(' '.join(['--directory ' + os.getcwd() + '/' + app + '/src' for app in options.application[1:]])).split()

    # Initialize Mode (run initialize mode before run_tests, or application)
    if options.mode == 'initialize':
        if len(options.application) > 1:
            tmp_cmd.append([options.lcov_command[0],
                            '--gcov-tool', options.cov_tool,
                            '--capture',
                            '--initial',
                            '--directory', os.getcwd() + '/' + options.application[0],
                            '--output-file', os.getcwd() + '/initialize.info'
                            ])
            tmp_cmd[0].extend(tmp_additional_directories)
        else:
            tmp_cmd.append([options.lcov_command[0],
                            '--gcov-tool', options.cov_tool,
                            '--capture',
                            '--initial',
                            '--directory', os.getcwd() + '/' + options.application[0],
                            '--output-file', os.getcwd() + '/initialize.info'
                            ])

    # Generate Mode (run generate mode only after initialize mode, run_tests/application has ran)
    if options.mode == 'generate':
        if len(options.application) > 1:
            tmp_cmd.append([options.lcov_command[0],
                            '--gcov-tool', options.cov_tool,
                            '--directory', os.getcwd() + '/' + options.application[0],
                            '--capture',
                            '--ignore-errors', 'gcov,source',
                            '--output-file', os.getcwd() + '/covered.info'
                            ])
            tmp_cmd[0].extend(tmp_additional_directories)
        else:
            tmp_cmd.append([options.lcov_command[0],
                            '--gcov-tool', options.cov_tool,
                            '--directory', os.getcwd() + '/' + options.application[0],
                            '--capture',
                            '--ignore-errors', 'gcov,source',
                            '--output-file', os.getcwd() + '/covered.info'
                            ])

        # Build lcov combine command
        tmp_cmd.append([options.lcov_command[0],
                        '--gcov-tool', options.cov_tool,
                        '--add-tracefile', os.getcwd() + '/initialize.info',
                        '--add-tracefile', os.getcwd() + '/covered.info',
                        '--output-file', os.getcwd() + '/combined.info' ])

        # Build lcov filter command
        tmp_cmd.append([options.lcov_command[0],
                        '--gcov-tool', options.cov_tool,
                        '--extract', os.getcwd() + '/combined.info', '*' + options.application[0] + '/src*',
                        '--extract', os.getcwd() + '/combined.info', '*' + options.application[0] + '/include*',
                        '--extract', os.getcwd() + '/combined.info', '*' + options.application[0] + '/build/header_symlinks*',
                        '--output-file', options.outfile ])

        # Build genhtml command if --generate-html was used
        if options.generate_html:
            tmp_cmd.append([options.genhtml_command[0], options.outfile,
                            '--title', options.title + ' Test Coverage',
                            '--num-spaces', '2',
                            '--legend',
                            '--no-branch-coverage',
                            '--output-directory', options.html_location])

        # Clean up old tracefiles if asked
        if options.cleanup:
            tmp_cmd.append(['rm', '-f', os.getcwd() + '/initialize.info', os.getcwd() + '/covered.info', os.getcwd() + '/combined.info'])

    # Combine Mode
    if options.mode == 'combine':
        # Build lcov tracefile command
        for single_tracefile in options.add_tracefile:
            options.lcov_command.extend(['--add-tracefile', single_tracefile])
        options.lcov_command.extend(['-o', options.outfile ])
        tmp_cmd.append(options.lcov_command)
        # Build genhtml command if --generate-html was used
        if options.generate_html:
            tmp_cmd.append([options.genhtml_command[0], options.outfile,
                            '--title', options.title + ' Test Coverage',
                            '--num-spaces', '2',
                            '--legend',
                            '--no-branch-coverage',
                            '--output-directory', options.html_location])

    # Sync Mode
    if options.mode == 'sync':
        tmp_cmd.append([options.rsync_command[0],
                        '-ro', options.html_location, options.sync_location ])

    # Run all built commands. This is where the magic happens
    for single_command in tmp_cmd:
        if options.debug:
            print('\n\nDry Run:', str(options.mode) + ' Mode\n', ' '.join(single_command))
        else:
            runCMD(single_command)

    # Run post-process stuff
    postProcess(options)

    # Run verify coverage (if enabled)
    if options.verify_coverage:
        verifyCoverage(options)

def verifyCoverage(options):
    summary_command = subprocess.Popen([options.lcov_command[0],
                                        '--gcov-tool',
                                        options.cov_tool,
                                        '--summary',
                                        options.outfile],
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE)

    stdout, stderr = summary_command.communicate()
    summary_output = ' '.join([stdout.decode(), stderr.decode()])
    coverage = float(re.findall(r'lines.*: (\d+.\d+)', summary_output)[0])
    print(summary_output, '\n\nCode Coverage:', str(coverage))
    if coverage >= options.coverage_percent:
        sys.exit(0)
    else:
        sys.exit(1)

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
                print('\n\nDry Run: deleting file', tracefile)
            else:
                try:
                    os.remove(tracefile)
                except:
                    pass
        if options.debug:
            print('\n\nDry Run: deleting file', options.outfile)
        else:
            try:
                os.remove(options.outfile)
            except:
                pass
    if options.mode == 'generate':
        if options.debug:
            print('\n\nDry Run: deleting file', options.outfile)
        else:
            try:
                os.remove(options.outfile)
            except:
                pass

def addBetterDate(options):
    if options.debug:
        print('\n\nDry Run: appending timestamp to generated HTML content\n', "echo '" + str(time.ctime(time.time())) + "'", '>>', str(options.html_location) + '/index.html')
    else:
        current_time = time.ctime(time.time())
        output_file = open(options.html_location + '/index.html', 'a')
        output_file.write(current_time)
        output_file.close()

def runCMD(cmd_opts):
    a_proc = subprocess.Popen(cmd_opts,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE)

    retstr = a_proc.communicate()
    if not a_proc.poll() == 0:
        print('Error:', retstr[1].decode())
        sys.exit(1)
    else:
        return retstr[0].decode()

def exit_error(error_list):
    print('\n\tThere were errors running build_coverage:\n')
    for sgl_error in error_list:
        print(sgl_error, '\n')
    sys.exit(1)

def _find(myfile, matchFunc=os.path.isfile):
    paths = os.getenv('PATH').split(os.pathsep)
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
        if options.outfile is not None and os.path.exists(options.outfile):
            if options.overwrite is False:
                error_list.append('output file specified already exists. Exiting for safty measures...')
        if options.outfile is None:
            error_list.append('you must specifiy an output file: --outfile')

    # generate specific parsing options
    if options.mode == 'initialize':
        if options.application is None:
            error_list.append('initialize mode requires a list of applications to zero counters: --application <list of directories>')

    # generate specific parsing options
    if options.mode == 'generate':
        if options.application is None:
            error_list.append('generate mode requires a list of applications to test: --application <list of directories>')

    # combine specific parsing options
    if options.mode == 'combine':
        if options.add_tracefile is None:
            error_list.append('combine mode requires a list of tracefiles to combine: --add-tracefile <list of tracefiles>')
        if options.outfile is None:
            error_list.append('you must specifiy an output file: --outfile')
        if options.outfile is not None and os.path.exists(options.outfile):
            if options.overwrite is False:
                error_list.append('output file specified already exists. Exiting for safty measures...')

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
    parser.add_argument('--verify-coverage', dest='verify_coverage', action='store_const', const=True, default=False, help='Verify if coverage succeeds 80 percent. \nstdout True|False, as well as exits with 0|1\n ')
    parser.add_argument('--title', help='Title name of code coverage generated HTML \noutput\n ')
    parser.add_argument('--outfile', help='Output tracefile\n ')
    parser.add_argument('--mode', choices=['generate','initialize','combine','sync'], help='Choose an operational mode:\n\nINITIALIZE: Initialize code coverage for each\nspecified --application\n\nGENERATE: Generate code coverage for each\nspecified --application\n\nCOMBINE: Combines tracefiles generated by \ndifferent runs of generate mode\n\nSYNC: Optionally rsync the data to a specified\nlocation. Requires --html-location and\n--sync-location\n ')
    parser.add_argument('--application', metavar='directories', nargs='+', help='A list of Application/s to cover (path to \ndirectories). Used in conjunction with\ngenerate mode.\n ')
    parser.add_argument('--add-tracefile', metavar='tracefiles', nargs='+', help='A list of tracefiles to use in conjunction\nwith combine mode.\n ')
    parser.add_argument('--html-location', help='Location of HTML generated content. Used in\nconjunction with --generate-html or --sync-location\n ')
    parser.add_argument('--sync-location', help='location to rsync the data to:\nuserid@server:/some/location\n ')
    parser.add_argument('--coverage-percentage', dest='coverage_percent', type=float, default=80.0, help='If specified, this is the percentage coverage has to pass\n ')
    parser.add_argument('--cov-tool', metavar='coverage_tool', default="gcov", help='Which coverage tool to use (gcov default)\n ')
    options = parser.parse_args(args)
    return _verifyOptions(options)

if __name__ == '__main__':
    options = _parseARGs()
    buildCMD(options)
