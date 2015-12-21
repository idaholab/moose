#!/usr/bin/env python
import cookielib
import sys, os, time, argparse, getpass, re

try:
  import mechanize
except ImportError:
  print 'python-mechanize module not available.\n'
  sys.exit(1)

def webBrowser():
  # Browser
  br = mechanize.Browser()

  # Cookie Jar
  cj = cookielib.LWPCookieJar()
  br.set_cookiejar(cj)

  # Browser options
  br.set_handle_equiv(True)
  br.set_handle_gzip(False)
  br.set_handle_redirect(True)
  br.set_handle_referer(True)
  br.set_handle_robots(False)
  br.set_handle_refresh(mechanize._http.HTTPRefreshProcessor(), max_time=1)
  br.addheaders = [('User-agent', 'Chrome')]

  # The site we will navigate into, handling it's session
  br.open('https://github.com/login')

  # Select the second (index one) form (the first form is a search query box)
  # this changes from web site to web site. GitHub.com/login happens to be the second form
  br.select_form(nr=0)
  return br

def authenticatePage(args):
  browser = webBrowser()
  browser.form['login'] = args.user
  browser.form['password'] = args.password
  browser.submit()
  return browser

def readPage(browser, args):
  stats = {}
  browser.addheaders = [('User-agent', 'Chrome'), ('Referer', 'https://github.com/' + args.repo + '/graphs/traffic'), ('X-Requested-With', 'XMLHttpRequest')]
  # GitHubs Traffic payload is in python dictionary format
  # grab the clones, and Visitors
  try:
    stats['Clones'] = eval(browser.open('https://github.com/' + args.repo + '/graphs/clone-activity-data').read())
    stats['Visitors'] = eval(browser.open('https://github.com/' + args.repo + '/graphs/traffic-data').read())
  except mechanize.HTTPError as e:
    print 'There was an error obtaining traffic for said site.'
    if str(e).find('406') != -1:
      print '\tError 406: You do not have permission to view statistics. Or you supplied incorrect credentials'
      sys.exit(1)
    if str(e).find('404') != -1:
      print '\tError 404: Page not found'
      sys.exit(1)
  return stats

def verifyArgs(args):
  if args.repo is None or len(args.repo.split('/')) != 2:
    print '\nYou must specify a repository you are insterested in scrapeing:\n\t --repo foo/bar\n\nNote: GitHub is case-sensitive, so your arguments must be too'
    sys.exit(1)
  if args.user is '':
    print '\nYou must specify a user to authenticate with'
    sys.exit(1)
  try:
    while args.password is '':
      args.password = getpass.getpass('Password for UserID ' + args.user + ' :',)
  except KeyboardInterrupt:
    print ''
    sys.exit(0)
  return args

def writeFile(args, stats):
  if os.path.isfile(args.write):
    log_file = open(args.write, 'r')
    file_data = log_file.read()
    log_file.close()
    # True if the file header contains the same name as --repo
    if args.repo == file_data.split('\n')[0]:
      old_clones = eval(re.findall(r'(\[.*\])', file_data.split('\n')[1])[0])
      old_visitors = eval(re.findall(r'(\[.*\])', file_data.split('\n')[2])[0])

      # Remove overlapping list items based on date in our old list. Store this old data in a new list
      tmp_clones = old_clones[old_clones.index(stats['clones'][len(stats['clones'])-3]) + 3:]
      tmp_visitors = old_visitors[old_visitors.index(stats['visitors'][len(stats['visitors'])-3]) + 3:]

      # Insert the new data at index 0 (GitHub reports newest items at the begining of the list) into our old list
      tmp_clones[0:0] = stats['clones']
      tmp_visitors[0:0] = stats['visitors']

      # write the new data into file, overwriting any previous data
      log_file = open(args.write, 'w')
      log_file.write(args.repo + '\nclones = ' + str(tmp_clones) + '\nvisitors = ' + str(tmp_visitors) + '\n')
      log_file.close()
      sys.exit(0)

    else:
      print 'The file you attempted to write to contains stats for another repository (' + file_data.split('\n')[0] + \
            ')\nwhile you supplied arguments to gather stats for (' + args.repo + \
            ').\n\n... Or this is probably not the file you wanted to overwrite:\n\t', args.write, '\nExiting just to be safe...\n'
      sys.exit(1)
  else:
    log_file = open(args.write, 'w')
    log_file.write(args.repo + '\nclones = ' + str(stats['clones']) + '\nvisitors = ' + str(stats['visitors']) + '\n')
    log_file.close()

def parseArgs(args=None):
  # Traffic Stats URL: https://github.com/idaholab/moose/graphs/clone-activity-data
  parser = argparse.ArgumentParser(description='Scrape GitHub for a webpage requiring authentication')
  parser.add_argument('--repo', '-r', nargs='?', help='Repository (example: foo/bar)')
  parser.add_argument('--write', '-w', nargs='?', help='Write to a file')
  try:
    parser.add_argument('--user', '-u', nargs='?', default=os.getenv('USER'), help='Authenticate using specified user. Defaults to: (' + os.getenv('USER') + ')')
  except TypeError:
    parser.add_argument('--user', '-u', nargs='?', default='', help='Authenticate using specified user')
  parser.add_argument('--password', '-p', nargs='?', default='', help='Authenticate using specified password')
  return verifyArgs(parser.parse_args(args))

if __name__ == '__main__':
  args = parseArgs()
  web_page = authenticatePage(args)
  payload = readPage(web_page, args)
  stats = {'clones'   : [],
           'visitors' : []}
  for point in payload['Clones']['counts']:
    stats['clones'].extend([time.strftime("%Y-%b-%d", time.gmtime(point['bucket'])), str(point['total']), str(point['unique'])])
  for point in payload['Visitors']['counts']:
    stats['visitors'].extend([time.strftime("%Y-%b-%d", time.gmtime(point['bucket'])), str(point['total']), str(point['unique'])])

  if args.write:
    writeFile(args, stats)
  else:
    print '\nClones: (date, total, unique)\n', stats['clones']
    print '\nVisitors: (date, total, unique)\n', stats['visitors']
