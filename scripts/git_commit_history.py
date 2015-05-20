#!/usr/bin/env python
import sys
import subprocess
import datetime
import re
import numpy
import pylab
import multiprocessing
import argparse

# A helper function for running git commands
def run(*args):
  output, _ = subprocess.Popen(args, stdout = subprocess.PIPE).communicate()
  return filter(None, output.split('\n'))

# Return the list of contributors, sorted by the number of contributions
def getContributors():

  # Extract the authors and total number of commits
  log = run('git', 'shortlog', '-s')
  authors = []
  commits = []
  for row in log:
    r = row.split('\t')
    commits.append(int(r[0]))
    authors.append(r[1])

  # Return the authors sorted by commit count
  return [x for (y,x) in sorted(zip(commits, authors), reverse=True)]

# Return the date and contribution date
def getData(contributors):

  # Build a list of unique dates
  dates = sorted(set(run('git', 'log', '--reverse', '--format=%ad', '--date=short')))


  # Build the data arrays, filled with zeros
  data = numpy.zeros((len(contributors), len(dates)), dtype=int)
  contrib = numpy.zeros(len(dates), dtype=int)
  unique_contributors = []

  # Get list of all commits with author and date
  commits = run('git', 'log', '--format=%aN,%ad', '--date=short', '--reverse')
  for commit in commits:
    c = commit.split(',')
    i = contributors.index(c[0]) # author index
    j = dates.index(c[1]) # date index
    data[i,j] += 1

    # Count unique contributions
    if contributors[i] not in unique_contributors:
      unique_contributors.append(contributors[i])
      contrib[j] += 1

  # Perform cumlative summations
  data = numpy.cumsum(data, axis=1)
  contrib = numpy.cumsum(contrib)

  # Convert the dates to datetime
  for i in range(len(dates)):
    dates[i] = datetime.datetime.strptime(dates[i], '%Y-%m-%d')

  # Return the data
  return dates, data, contrib

# MAIN
if __name__ == '__main__':

  # Command-line options
  parser = argparse.ArgumentParser(description="Tool for building commit history of a git repository")
  parser.add_argument('--stack', '-s', action='store_true', help='Show graph as stacked area instead of line plot')
  parser.add_argument('--unique', '-u', action='store_true', help='Show unique contributor on secondary axis')
  parser.add_argument('--open-source', '-r', action='store_true', help='Show shaded region for open sourcing of MOOSE')
  parser.add_argument('--pdf', '--file', '-f', action='store_true', help='Write the plot to a pdf file (see --output)')
  parser.add_argument('--output', '-o', type=str, default='commit_history.pdf', help='The filename for writting the plot to a file')
  parser.parse_args('-surf'.split())
  options = parser.parse_args()

  # Build a list of contributors
  contributors = getContributors()

  # Extract the data
  dates, data, contrib = getData(contributors)

  # Create the figure
  fig, ax1 = pylab.subplots()

  # Plot the data
  if options.stack: # stack plot
    handles = pylab.stackplot(dates, data)
    for i in range(len(handles)):
      handles[i].set_label(contributors[i])

  else: # line plot
    nx,ny = data.shape
    handles = [None]*nx
    for i in range(nx):
      handles[i] = ax1.plot(dates, data[i,:], label=contributors[i])

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
    ax2.plot(dates, contrib, linewidth=3)
    ax2.set_ylabel('Unique Contributors', color='b')

  # Write to a file
  if options.pdf:
    fig.savefig(options.output)

  pylab.show()
