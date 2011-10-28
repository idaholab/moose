#!/usr/bin/python
import sys, string, subprocess, re, socket

checkout_moose_stable = ['svn', 'co', 'svn+ssh://hpcsc/svn/herd/branches/stable/moose', 'moose-stable']
get_merged_revisions = ['svn', 'mergeinfo', 'svn+ssh://hpcsc/svn/herd/trunk/moose', '--show-revs', 'eligible', 'moose-stable']
get_revision_logs = ['svn', 'log']
commit_moose_stable = ['svn', 'ci', '--username', 'moosetest', 'moose-stable', '-m']

def runCMD(cmd_opts):
  a_proc = subprocess.Popen(cmd_opts, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  retstr = a_proc.communicate()
  if not a_proc.poll() == 0:
    return [ False, retstr[1] ]
  else:
    return [ True, retstr[0] ]

def parseLOG(merge_log):
  svn_log = []
  final_log = ''
  merge_list = re.split('\n+', merge_log)
  for item in merge_list:
    if re.match(r'(^-+)', item) is not None:
      svn_log.append('  ----\n')
    else:
      for cmd_language in ['close #', 'closed #', 'closes #', 'fix #', 'fixed #', 'fixes #', 'references #', 'ref #', 'refs #', 'addresses #', 're #', 'see #']:
        tmp_item = str.lower(item)
        if tmp_item.find(cmd_language) != -1:
          pos_start = (int(tmp_item.find(cmd_language)) + (len(cmd_language) - 2))
          pos_end = (int(tmp_item.find(cmd_language)) + len(cmd_language))
          item = item[:pos_start] + ': #' + item[pos_end:]
      svn_log.append(item + '\n')
  for log_line in svn_log:
    final_log = final_log + str(log_line)
  return final_log

def clobberRevisions(revision_list):
  tmp_list = ''
  for item in revision_list:
    if item != '':
      tmp_list = tmp_list + ' -' + item
  return tmp_list

if __name__ == '__main__':
  if socket.gethostname().split('.')[0] == 'quark':
    runCMD(checkout_moose_stable)
    merged_revisions = runCMD(get_merged_revisions)
    if merged_revisions[0]:
      merged_revisions = string.split(merged_revisions[1], '\n')
    for revision in merged_revisions:
      if revision != '':
        get_revision_logs.append('-' + revision)
    get_revision_logs.append('svn+ssh://hpcsc/svn/herd/trunk/moose')
    merge_log = runCMD(get_revision_logs)
    final_log = parseLOG(merge_log[1])
    commit_moose_stable.append('$\'' + final_log + '\'')
    runCMD(commit_moost_stable)
