#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import argparse, sys, os, re
import datetime
try:
    import requests
except ImportError:
    print('python-requests module not available.\n')
    sys.exit(1)


def getClones(args):
    github = 'https://api.github.com/repos'
    urls = { 'clones' : '/'.join([github, args.repo, 'traffic/clones']),
             'views'  : '/'.join([github, args.repo, 'traffic/views'])}

    headers = {'Authorization': 'token ' + args.token,
               'Accept': 'application/vnd.github.spiderman-preview'}
    params = {}
    results = {}

    if args.per_week:
        params = {'per' : 'week'}

    for search_type, url in urls.iteritems():
        r = requests.get(url, params=params, headers=headers)
        if r.status_code != 200:
            print('There was an error while attempting to gather data:', r.status_code, '\n', r.text)
            sys.exit(1)
        else:
            results[search_type] = r.json()
    return results


def fixTime(iso_time):
    tmp_time = datetime.datetime.strptime(iso_time, "%Y-%m-%dT%H:%M:%SZ")
    return tmp_time.strftime('%Y-%b-%d')

def parseData(search_type, json_data):
    # The new GitHub API reversed the list from our previous scrapping method
    json_data[search_type].reverse()

    activity = []
    for item in json_data[search_type]:
        formatted_time = fixTime(item['timestamp'])
        total = str(item['count'])
        unique = str(item['uniques'])
        activity.extend([formatted_time, total, unique])
    return activity

def writeFile(args, stats):
    if os.path.isfile(args.write):
        with open(args.write, 'r') as log_file:
            file_data = log_file.read()
        # True if the file header contains the same name as --repo
        if args.repo == file_data.split('\n')[0]:
            old_clones = eval(re.findall(r'(\[.*\])', file_data.split('\n')[1])[0])
            old_visitors = eval(re.findall(r'(\[.*\])', file_data.split('\n')[2])[0])

            # Remove overlapping list items based on date in our old list. Store this old data in a new list
            tmp_clones = old_clones[old_clones.index(stats['clones'][len(stats['clones'])-3]) + 3:]
            tmp_visitors = old_visitors[old_visitors.index(stats['views'][len(stats['views'])-3]) + 3:]

            # Insert the new data at index 0 (GitHub reports newest items at the begining of the list) into our old list
            tmp_clones[0:0] = stats['clones']
            tmp_visitors[0:0] = stats['views']

            # write the new data into file, overwriting any previous data
            log_file = open(args.write, 'w')
            log_file.write(args.repo + '\nclones = ' + str(tmp_clones) + '\nvisitors = ' + str(tmp_visitors) + '\n')
            log_file.close()
            sys.exit(0)

        else:
            print('The file you attempted to write to contains stats for another repository (' + file_data.split('\n')[0] + \
                  ')\nwhile you supplied arguments to gather stats for (' + args.repo + \
                  ').\n\n... Or this is probably not the file you wanted to overwrite:\n\t', args.write, '\nExiting just to be safe...\n')
            sys.exit(1)
    else:
        with open(args.write, 'w') as log_file:
            log_file.write(args.repo + '\nclones = ' + str(stats['clones']) + '\nvisitors = ' + str(stats['views']) + '\n')

def verifyArgs(parser, args):
    if args.token is None or args.repo is None:
        print('you must supply a repository and token to interface with\n')
        parser.print_help()
        sys.exit(1)
    return args

def parseArgs(args=None):
    parser = argparse.ArgumentParser(description="Obtain cloning information using GitHub's API")
    parser.add_argument("--repo", "-r", nargs="?", help="Repository (example: foo/bar)")
    parser.add_argument("--write", "-w", nargs="?", help="Write results to a file")
    parser.add_argument("--token", "-t", nargs="?", default="", help="Authenticate using specified token. Token must have admin priviledges for the 'repo' scope in your user settings.")
    parser.add_argument("--per-week", action='store_const', const=True, default=False, help="Obtain clones per week instead of day")
    return verifyArgs(parser, parser.parse_args(args))

if __name__ == "__main__":
    options = parseArgs()
    results = getClones(options)
    formatted_results = {}
    for search_type, data in results.iteritems():
        formatted_results[search_type] = parseData(search_type, data)
    if options.write:
        writeFile(options, formatted_results)
    else:
        print(formatted_results)
