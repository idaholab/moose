#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import subprocess
import datetime
import re
import numpy
import matplotlib.pyplot as plt
import matplotlib
import multiprocessing
import argparse
import itertools
import math
import os

##############################
# Favorite plots
# $ ./git_commit_history.py --open-source --authors=all --unique --disable-legend --output=contributors.pdf
# $ ./git_commit_history.py --additions --authors=all --days=7 --open-source  --disable-legend --output=additions.pdf
##############################

from matplotlib import rc

# A helper function for running git commands
def run(*args, **kwargs):

    options = kwargs.pop("options", None)

    if options:
        loc = os.getenv('MOOSE_DIR', os.path.join(os.getenv('HOME'), 'projects', 'moose'))
        if options.framework:
            loc = os.path.join(loc, 'framework')
        elif options.modules:
            loc = os.path.join(loc, 'modules')
        args += ('--', loc)

    output = subprocess.check_output(args, encoding='utf8').strip()
    if kwargs.pop('split', True):
        return output.split('\n')
    else:
        return output

# Return the list of contributors, sorted by the number of contributions
def getContributors(options, **kwargs):

    # Get the number of authors
    num_authors = kwargs.pop('authors', options.authors)

    # Extract the authors and total number of commits
    log = run('git', 'shortlog', '-s', '--no-merges', options=options)
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
        contributors = ['Derek Gaston', 'Cody Permann', 'David Andrs', 'John W. Peterson', 'Andrew E. Slaughter', 'Brain Alger', 'Fande Kong', 'Robert Carlsen', 'Alex Lindsay', 'Jason M. Miller']
        contributors += ['Other (' + str(n-len(contributors)) + ')']

    elif num_authors == 'all':
        contributors = ['All']

    elif num_authors:
        num_authors = int(num_authors)
        contributors = contributors[0:num_authors]
        contributors += ['Other (' + str(n-num_authors) + ')']

    return contributors

# Return the date and contribution date
def getData(options):

    # Build a list of contributors
    contributors = getContributors(options)

    # Flag for lumping into two categories MOOSE developers and non-moose developers
    dev = options.moose_dev
    if dev:
        moose_developers = contributors[0:-1]
        contributors = ['MOOSE developers (' + str(len(contributors)-1) + ')', contributors[-1]]

    # Build a list of unique dates
    all_dates = sorted(set(run('git', 'log', '--reverse', '--format=%ad', '--date=short', options=options)))
    d1 = datetime.datetime.strptime(all_dates[0], '%Y-%m-%d')
    d2 = datetime.datetime.strptime(all_dates[-1], '%Y-%m-%d')
    dates = [d1 + datetime.timedelta(days=x) for x in range(0, (d2-d1).days, options.days)]

    # Build the data arrays, filled with zeros
    N = numpy.zeros((len(contributors), len(dates)), dtype=int)
    data = {'commits' : numpy.zeros((len(contributors), len(dates)), dtype=int),
            'in' : numpy.zeros((len(contributors), len(dates)), dtype=int),
            'out' : numpy.zeros((len(contributors), len(dates)), dtype=int)}

    contrib = numpy.zeros(len(dates), dtype=int)
    all_contributors = getContributors(options, authors=None)
    unique_contributors = []

    # Get the additions/deletions
    commits = run('git', 'log', '--format=%H\n%ad\n%aN', '--date=short', '--no-merges', '--reverse', '--shortstat', split=False, options=options)
    commits = [x for x in re.split(r'[0-9a-z]{40}', commits) if x]

    # Loop over commits
    for commit in commits:
        c = commit.strip().split('\n')
        date = datetime.datetime.strptime(c[0], '%Y-%m-%d')
        author = c[1]

        if dev and author in moose_developers:
            author = contributors[0]
        elif author not in contributors:
            author = contributors[-1]

        i = contributors.index(author) # author index

        d = [x for x in dates if x > date]
        if d:
            j = dates.index(d[0])
        else:
            j = dates.index(dates[-1])
        data['commits'][i,j] += 1

        if options.additions:
            a = c[3].split()
            n = len(a)
            files = int(a[0])

            if n == 5:
                if 'insertion' in a[4]:
                    plus = int(a[3])
                    minus = 0
                else:
                    minus = int(a[3])
                    plus = 0
            else:
                plus = int(a[3])
                minus = int(a[5])

            data['in'][i,j] += plus
            data['out'][i,j] += minus

        # Count unique contributions
        unique_author_index = all_contributors.index(c[1])
        unique_author = all_contributors[unique_author_index]
        if unique_author not in unique_contributors:
            unique_contributors.append(unique_author)
            contrib[j] += 1

    # Perform cumulative summations
    data['commits'] = numpy.cumsum(data['commits'], axis=1)
    contrib = numpy.cumsum(contrib)

    # Return the data
    return dates, data, contrib, contributors


# MAIN
if __name__ == '__main__':

    # Command-line options
    parser = argparse.ArgumentParser(description="Tool for building commit history of a git repository")
    parser.add_argument('--additions', action='store_true', help='Show additions/deletions graph')
    parser.add_argument('--days', type=int, default=1, help='The number of days to lump data (e.g., use 7 for weekly data)')
    parser.add_argument('--disable-legend', action='store_true', help='Disable display of legend')
    parser.add_argument('--stack', '-s', action='store_true', help='Show graph as stacked area instead of line plot')
    parser.add_argument('--unique', '-u', action='store_true', help='Show unique contributor on secondary axis')
    parser.add_argument('--unique-label', default='legend', choices=['none', 'arrow', 'legend'], help='Control how the unique contributor line is labeled.')
    parser.add_argument('--open-source', '-r', action='store_true', help='Show shaded region for open sourcing of MOOSE')
    parser.add_argument('--output', '-o', type=str, default='commit_history.pdf', help='The filename for writing the plot to a file')
    parser.add_argument('--authors', default=None, help='Limit the graph to the given number of entries authors, or use "moose" to limit to MOOSE developers')
    parser.add_argument('--moose-dev', action='store_true', help='Create two categories: MOOSE developers and other (this overrides --authors)')
    parser.add_argument('--framework', action='store_true', help='Limit the analysis to framework directory')
    parser.add_argument('--modules', action='store_true', help='Limit the analysis to modules directory')
    parser.add_argument('--font', default=12, help='The font-size, in points')
    parser.parse_args('-sur'.split())
    options = parser.parse_args()

    # Markers/colors
    marker = itertools.cycle(('o', 'v', 's', 'd'))
    color = itertools.cycle(('r', 'b', 'g', 'c', 'm', 'y', 'k'))

    # Setup authors defaults for various cases
    if options.moose_dev and options.authors:
        raise Exception("Can not specify both --authors and --moose-dev");
    elif options.moose_dev:
        options.authors = 'moose'

    # Error if both --framework and --modules are given
    if options.framework and options.modules:
        raise Exception("Can not specify both --framework and --modules")

    # Extract the data
    dates, data, contrib, contributors = getData(options)

    # Create the figure
    fig, ax1 = plt.subplots(dpi=200)
    fig.set_facecolor([1,1,1])
    for tick in ax1.yaxis.get_ticklabels():
        tick.set_fontsize(options.font)
    for tick in ax1.xaxis.get_ticklabels():
        tick.set_fontsize(options.font)

    # Show unique contributors
    if options.unique:
        ax2 = ax1.twinx()
        ax2.plot(dates, contrib, linewidth=2, linestyle='-', color='royalblue', label='Unique Contributors')
        ax2.set_ylabel('Unique Contributors', color='royalblue', fontsize=options.font)
        for tick in ax2.xaxis.get_ticklabels() + ax2.yaxis.get_ticklabels():
            tick.set_fontsize(options.font)
            tick.set_color('royalblue')

        if options.unique_label == 'arrow':
            arrow = dict(arrowstyle="-|>", connectionstyle="arc3,rad=0.3", fc="w")
            i = int(len(dates)*0.75)
            c = int(contrib[-1]*0.75)
            ax2.annotate('Unique Contributors', xy=(dates[i], contrib[i]), xytext=(datetime.date(2014,1,1), c), ha='right', size=options.font, arrowprops=arrow)

    # labels
    y_label = 'Commits'

    # Plot the data
    if options.stack: # stack plot
        handles = plt.stackplot(dates, data['commits'])
        for i in range(len(handles)):
            handles[i].set_label(contributors[i])

    elif options.additions: #additions/deletions plot
        y_label = 'Lines Added / Deleted'
        n = len(contributors)
        for i in reversed(range(n)):
            x = numpy.array(dates)
            y = numpy.log10(numpy.array(data['in'][i,:]))

            if n == 1:
                label = 'Additions'
                alpha = 0.95
                clr = 'b'
            else:
                label = contributors[i] + '(Additions)'
                alpha = 0.4
                clr = next(color)

            ax1.fill_between(x, 0, y, label=label, alpha=alpha, color=clr)

            y = -numpy.log10(numpy.array(data['out'][i,:]))

            if n == 1:
                label = 'Deletions'
                clr = 'r'
            else:
                label = contributors[i] + '(Deletions)'
                clr = next(color)

            ax1.fill_between(x, 0, y, label=label, alpha=alpha, color=clr)

        fig.canvas.draw()

        labels = []
        for value in ax1.get_yticks():
            if value < 0:
                labels.append('$-10^{}$'.format(numpy.abs(int(value))))
            else:
                labels.append('$10^{}$'.format(int(value)))

        ax1.set_yticklabels(labels)
        ax1.grid()

        if not options.disable_legend:
            handles, labels = ax1.get_legend_handles_labels()

            if options.unique and options.unique_label== 'legend':
                h, l = ax2.get_legend_handles_labels()
                handles.append(h[0])
                labels.append(l[0])

            lgnd = plt.legend(handles, labels, loc='upper left', fontsize=options.font)
            lgnd.draw_frame(False)

    else: # line plot
        handles = []
        for i in range(len(contributors)):
            x = numpy.array(dates)
            y = data['commits'][i,:]
            idx = y>0
            clr = 'k' if len(contributors) == 1 else next(color)
            mkr = None if len(contributors) == 1 else next(marker)
            h = ax1.plot(x[idx], y[idx], label=contributors[i], linewidth=2, markevery=60, marker=mkr, color=clr)
            handles.append(h[0])

        if len(contributors) == 1:
            y_label = 'Total Contributions'

        if not options.disable_legend:
            if options.unique and options.unique_label== 'legend':
                h, l = ax2.get_legend_handles_labels()
                handles.append(h[0])
                contributors.append(l[0])

            lgnd = plt.legend(handles, contributors, loc='upper left', fontsize=options.font)
            lgnd.draw_frame(False)

    # Add labels
    ax1.set_ylabel(y_label, fontsize=options.font)
    ax1.set_xlabel('Date', fontsize=options.font)

    # Show open-source region
    if options.open_source:
        os_start = matplotlib.dates.date2num(datetime.date(2014,3,10))
        x_lim = ax1.get_xlim()
        if options.unique:
            y_lim = ax2.get_ylim()
        else:
            y_lim = ax1.get_ylim()

        delta = x_lim[1] - os_start
        plt.gca().add_patch(plt.Rectangle((os_start, y_lim[0]), delta, y_lim[1]-y_lim[0], facecolor='green', alpha=0.1))
        ax1.annotate('open source ', xy=(x_lim[1] - (delta/2.), y_lim[0]), ha='center', va='bottom', size=options.font)

    plt.tight_layout()
    plt.savefig(options.output, format='pdf')
