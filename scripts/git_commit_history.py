#!/usr/bin/env python
import sys
import subprocess
import datetime
import re
import numpy
import pylab
import multiprocessing
import argparse
import itertools

# A helper function for running git commands
def run(*args):
  output, _ = subprocess.Popen(args, stdout = subprocess.PIPE).communicate()
  return filter(None, output.split('\n'))

# Return the list of contributors, sorted by the number of contributions
def getContributors(num_authors):

  # Extract the authors and total number of commits
  log = run('git', 'shortlog', '-s', '--no-merges')
  authors = []
  commits = []
  for row in log:
    r = row.split('\t')
    commits.append(int(r[0]))
    authors.append(r[1])

  # Return the authors sorted by commit count
  contributors = [x for (y,x) in sorted(zip(commits, authors), reverse=True)]

  # Limit to the supplied number of authors
  n = len(contributors)
  if num_authors == 'moose':
    contributors = ['Derek Gaston', 'Cody Permann', 'David Andrs', 'John W. Peterson', 'Andrew E. Slaughter']
    contributors += ['Other (' + str(n-len(contributors)) + ')']

  elif num_authors:
    num_authors = int(num_authors)
    contributors = contributors[0:num_authors]
    contributors += ['Other (' + str(n-num_authors) + ')']

  return contributors

# Return the date and contribution date
def getData(options):

  # Build a list of contributors
  contributors = getContributors(options.authors)

  # Flag for lumping into two categories MOOSE developers and non-moose developers
  dev = options.moose_dev
  if dev:
    moose_developers = contributors[0:-1]
    contributors = ['MOOSE developers (' + str(len(contributors)-1) + ')', contributors[-1]]

  # Build a list of unique dates
  dates = sorted(set(run('git', 'log', '--reverse', '--format=%ad', '--date=short')))

  # Build the data arrays, filled with zeros
  data = numpy.zeros((len(contributors), len(dates)), dtype=int)
  contrib = numpy.zeros(len(dates), dtype=int)
  all_contributors = getContributors(None)
  unique_contributors = []

  # Get list of all commits with author and date
  commits = run('git', 'log', '--format=%aN,%ad', '--date=short', '--reverse', '--no-merges')
  for commit in commits:
    c = commit.split(',')
    author = c[0]
    date = c[1]

    if dev and author in moose_developers:
      author = contributors[0]
    elif author not in contributors:
      author = contributors[-1]

    i = contributors.index(author) # author index
    j = dates.index(date) # date index
    data[i,j] += 1

    # Count unique contributions
    unique_author_index = all_contributors.index(c[0])
    unique_author = all_contributors[unique_author_index]
    if unique_author not in unique_contributors:
      unique_contributors.append(unique_author)
      contrib[j] += 1

  # Perform cumulative summations
  data = numpy.cumsum(data, axis=1)
  contrib = numpy.cumsum(contrib)

  # Convert the dates to datetime
  for i in range(len(dates)):
    dates[i] = datetime.datetime.strptime(dates[i], '%Y-%m-%d')

  # Return the data
  return dates, data, contrib, contributors

# MAIN
if __name__ == '__main__':

  # Command-line options
  parser = argparse.ArgumentParser(description="Tool for building commit history of a git repository")
  parser.add_argument('--stack', '-s', action='store_true', help='Show graph as stacked area instead of line plot')
  parser.add_argument('--unique', '-u', action='store_true', help='Show unique contributor on secondary axis')
  parser.add_argument('--open-source', '-r', action='store_true', help='Show shaded region for open sourcing of MOOSE')
  parser.add_argument('--pdf', '--file', '-f', action='store_true', help='Write the plot to a pdf file (see --output)')
  parser.add_argument('--output', '-o', type=str, default='commit_history.pdf', help='The filename for writting the plot to a file')
  parser.add_argument('--authors', default=None, help='Limit the graph to the given number of entries authors, or use "moose" to limit to MOOSE developers')
  parser.add_argument('--moose-dev', action='store_true', help='Create two categories: MOOSE developers and other (this overrides --authors)')
  parser.parse_args('-surf'.split())
  options = parser.parse_args()

  # Markers/colors
  marker = itertools.cycle(('o', 'v', 's', 'd'))
  color = itertools.cycle(('b', 'g', 'r', 'c', 'm', 'y', 'k'))

  # Handle MOOSE developers option
  if options.moose_dev and options.authors:
    raise Exception("Can not specify both --authors and --moose-dev");
  elif options.moose_dev:
    options.authors = 'moose'

  # Extract the data
  dates, data, contrib, contributors = getData(options)

  # Create the figure
  fig, ax1 = pylab.subplots()

  # Plot the data
  if options.stack: # stack plot
    handles = pylab.stackplot(dates, data)
    for i in range(len(handles)):
      handles[i].set_label(contributors[i])

  else: # line plot
    handles = []
    for i in range(len(contributors)):
      x = numpy.array(dates)
      y = data[i,:]
      idx = y>0
      h = ax1.plot(x[idx], y[idx], label=contributors[i], linewidth=2, markevery=60, marker=marker.next(), color=color.next())
      handles.append(h[0])
    lgnd = pylab.legend(handles, contributors, loc='upper left')
    lgnd.draw_frame(False)

  # Add labels
  ax1.set_ylabel('Commits')
  ax1.set_xlabel('Date')

  # Show open-source region
  if options.open_source:
    os = datetime.date(2014,3,10)
    y_lim = pylab.ylim()
    delta = pylab.xlim()[1] - os.toordinal()
    pylab.gca().add_patch(pylab.Rectangle((os, y_lim[0]), delta, y_lim[1], facecolor='green', alpha=0.2))

  # Show unique contributors
  if options.unique:
    ax2 = ax1.twinx()
    ax2.plot(dates, contrib, linewidth=4, linestyle='-', color='k')
    ax2.set_ylabel('Unique Contributors', color='k')

    arrow = dict(arrowstyle="-|>", connectionstyle="arc3,rad=0.3", fc="w")
    ax2.annotate('Unique Contributors', xy=(dates[900], contrib[900]), xytext=(datetime.date(2014,1,1), 70), ha='right', size=12, arrowprops=arrow)

  # Write to a file
  if options.pdf:
    fig.savefig(options.output)

  pylab.show()
