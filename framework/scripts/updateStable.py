#!/usr/bin/python
import os, sys, string, subprocess, re, socket, getopt

# If hostname equals head_node, this script will run
head_node = 'quark'

# Moose stable and devel checkout locations
moose_stable = 'https://hpcsc.inl.gov/svn/herd/trunk/moose'
moose_devel = 'https://hpcsc.inl.gov/svn/herd/trunk/devel/moose'

# We exclude these applications:
excluded_applications = set(['r7_moose', 'rattlesnake', 'moose_unit'])

# Comment Syntax Coverage command:
comment_syntax_cmd = [ 'moose/contrib/nsiqcppstyle/nsiqcppstyle', '--quiet', '--basedir=moose', '-f', 'moose/contrib/nsiqcppstyle/syntax_style', '--output=html', '--url=https://hpcsc.inl.gov/moose/browser/trunk', '-o', 'output.html', 'moose']
rsync_comment_syntax_cmd = ['/usr/bin/rsync', '-av', '--delete', 'output.html', os.getenv('TRAC_SERVER') + ':/srv/www/ssl/MOOSE/coverage/' ]

_USAGE = """
updateStable.py repo_revision

Where repo_revision is the target merge revision.

"""

def buildStatus():
  tmp_apps = []
  tmp_passed = []
  # Open line itemed list of applications passing their tests
  log_file = open('moose/test_results.log', 'r')
  tmp_passed = string.split(log_file.read(), '\n')
  log_file.close()
  # Remove trailing \n element which creates an empty item
  tmp_passed.pop()
  # Get a list of applications tested, by searching each directory presently containing a run_test application
  for app_dir in os.listdir('.'):
    if os.path.exists(os.path.join(os.getcwd(), app_dir, 'run_tests')):
      tmp_apps.append(app_dir)
  # Return boolean if all application tests passed
  if len(((set(tmp_apps) - excluded_applications) - set(tmp_passed))) != 0:
    print 'Failing tests:', string.join(((set(tmp_apps) - excluded_applications) - set(tmp_passed)))
    return False
  else:
    return True

def getCoverage():
  # A list of stuff we don't want to include in code coverage. Add more here if needed (wild cards accepted).
  filter_out = [ 'contrib/mtwist*',
                 '/usr/include*',
                 '*/openmpi/*',
                 '*/libmesh/*'
                 ]
  # Use the same commands from the coverage_html script to generate the raw.info file
  coverage_cmd = [ 'lcov',
                   '--base-directory', 'moose',
                   '--directory', 'moose/src/',
                   '--capture',
                   '--ignore-errors', 'gcov,source',
                   '--output-file', 'raw.info'
                   ]
  # Put the lcov filtering command together
  filter_cmd = ['lcov']
  for sgl_filter in filter_out:
    filter_cmd.extend(['-r', 'raw.info', sgl_filter])
  filter_cmd.extend(['-o', 'moose.info'])
  # Generate the raw.info
  runCMD(coverage_cmd, True)
  # Generate the moose.info (a filtered list of actual code coverage were after)
  coverage_results = runCMD(filter_cmd, True)
  # Find the percentage graciously givin to us by our filter_cmd:
  coverage_score = coverage_results[(coverage_results.find('lines......: ') + 13):coverage_results.find('% ')]
  # Return the results
  if float(coverage_score) <= 80.0:
    print 'Failed Code Coverage: ' + str(coverage_score)
    return False
  else:
    print 'Succeeded Code Coverage: ' + str(coverage_score)
    return True


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

def parseLOG(merge_log):
  svn_log = []
  final_log = ''
  merge_list = re.split('\n+', merge_log)
  for item in merge_list:
    if re.match(r'(^-+)', item) != None:
      svn_log.append('  ----\n')
    else:
      tmp_item = str.lower(item)
      for cmd_language in ['close #', 'closed #', 'closes #', 'fix #', 'fixed #', 'fixes #', 'references #', 'refs #', 'addresses #', 're #', 'see #', 'close: #', 'closed: #', 'closes: #', 'fix: #', 'fixed: #', 'fixes: #', 'references: #', 'refs: #', 'addresses: #', 're: #', 'see: #']:
        if tmp_item.find(cmd_language) != -1:
          pos_start = (int(tmp_item.find(cmd_language)) + (len(cmd_language) - 2))
          pos_end = (int(tmp_item.find(cmd_language)) + (len(cmd_language) - 1))
          item = str(item[:pos_start]) + '-' + str(item[pos_end:])
      svn_log.append(item + '\n')
  for log_line in svn_log:
    final_log = final_log + str(log_line)
  return final_log

def writeLog(message):
  log_file = open('svn_log.log', 'w')
  log_file.write(message)
  log_file.close()

def clobberRevisions(revision_list):
  tmp_list = ''
  for item in revision_list:
    if item != '':
      tmp_list = tmp_list + ' -' + item
  return tmp_list

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
  return opts[0]

if __name__ == '__main__':
  if socket.gethostname().split('.')[0] in head_node:
    runCMD(comment_syntax_cmd)
    runCMD(rsync_comment_syntax_cmd)
    arg_revision = process_args()
    coverage_status = getCoverage()
    if buildStatus() and coverage_status:
      # Checking out moose-stable
      checkout_moose_stable = ['svn', 'co', '--quiet', moose_stable, 'moose-stable']
      runCMD(checkout_moose_stable)
      # Get Merged version numbers
      print 'Get revisions merged...'
      get_merged_revisions = ['svn', 'mergeinfo', moose_devel, '--show-revs', 'eligible', 'moose-stable']
      log_versions = runCMD(get_merged_revisions)
      # Group the revisions together and build our 'svn log -r' command
      get_revision_logs = ['svn', 'log' ]
      merged_revisions = string.split(log_versions, '\n')
      if merged_revisions[0] != '':
        for revision in merged_revisions:
          if revision != '' and int(revision.split('r')[1]) <= int(arg_revision) and int(revision.split('r')[1]) != 1:
            get_revision_logs.append('-' + revision)
      else:
        print 'I detect no merge information... strange.'
        sys.exit(1)
      # Get each revision log
      print 'Getting each log for revision merged...'
      get_revision_logs.append(moose_devel)
      log_data = runCMD(get_revision_logs)
      # Parse through and write the log file with out any command langauge present
      writeLog(parseLOG(log_data))
      # Merge our local created moose-stable with moose-trunk
      print 'Merging moose-stable from moose-devel only to the revision at which bitten was commanded to checkout'
      merge_moose_trunk = ['svn', 'merge', '-r1:' + str(arg_revision), moose_devel, 'moose-stable' ]
      runCMD(merge_moose_trunk)
      # Commit the changes!
      print 'Commiting merged moose-stable'
      commit_moose_stable = ['svn', 'ci', '--username', 'moosetest', '-F', 'svn_log.log', 'moose-stable']
      runCMD(commit_moose_stable)
    else:
      # This is the system 'head_node', but buildStatus() returned False... so exit as an error
      sys.exit(1)
  else:
    # This is not one of the systems in 'head_node', so exit normally
    sys.exit(0)
