#!/usr/bin/python
import os, sys, string, subprocess, re, socket

# If hostname equals head_node, this script will run
head_node = 'quark'

# We exclude these applications:
excluded_applications = set(['r7_moose', 'rattlesnake'])

# The checkout command
checkout_moose_stable = ['svn', 'co', '--quiet', 'https://hpcsc/svn/herd/branches/stable/moose', 'moose-stable']

# The mergeinfo command
get_merged_revisions = ['svn', 'mergeinfo', 'https://hpcsc/svn/herd/trunk/moose', '--show-revs', 'eligible', 'moose-stable']

# The merge command
merge_moose_trunk = ['svn', 'merge', 'https://hpcsc/svn/herd/trunk/moose', 'moose-stable' ]

# The final stable checkin command
commit_moose_stable = ['svn', 'ci', '--username', 'moosetest', '-F', 'svn_log.log', 'moose-stable']

# Ignor this line. It is just needed to build the initial command. More is appended to it later
get_revision_logs = ['svn', 'log']

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

def runCMD(cmd_opts):
  a_proc = subprocess.Popen(cmd_opts, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  retstr = a_proc.communicate()
  if not a_proc.poll() == 0:
    print 'Error:', retstr[1]
    sys.exit(1)
  else:
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
      for cmd_language in ['close #', 'closed #', 'closes #', 'fix #', 'fixed #', 'fixes #', 'references #', 'refs #', 'addresses #', 're #', 'see #']:
        if tmp_item.find(cmd_language) != -1:
          pos_start = (int(tmp_item.find(cmd_language)) + (len(cmd_language) - 2))
          pos_end = (int(tmp_item.find(cmd_language)) + (len(cmd_language) - 2))
          item = str(item[:pos_start]) + ':' + str(item[pos_end:])
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

if __name__ == '__main__':
  if socket.gethostname().split('.')[0] == head_node:
    if buildStatus():
      # Checking out moose-stable
      runCMD(checkout_moose_stable)
      # Get Merged version numbers
      print 'Get revisions merged...'
      log_versions = runCMD(get_merged_revisions)
      # Group the revisions together and build our 'svn log -r' command
      merged_revisions = string.split(log_versions, '\n')
      if merged_revisions[0] != '':
        for revision in merged_revisions:
          if revision != '':
            get_revision_logs.append('-' + revision)
      else:
        print 'I detect no merge information... strange.'
        sys.exit(1)
      get_revision_logs.append('https://hpcsc/svn/herd/trunk/moose')
      # Get each revision log
      print 'Getting each log for revision merged...'
      log_data = runCMD(get_revision_logs)
      # Parse through and write the log file with out any command langauge present
      writeLog(parseLOG(log_data))
      # Merge our local created moose-stable with moose-trunk
      print 'Merging moose-stable'
      runCMD(merge_moose_trunk)
      # Commit the changes!
      print 'Commiting merged moose-stable'
      runCMD(commit_moose_stable)
    else:
      # This is the system 'head_node', but buildStatus() returned False... so exit as an error
      sys.exit(1)
  else:
    # This is not the system 'head_node', so exit normally
    sys.exit(0)
