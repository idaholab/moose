#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import datetime
import time
import argparse
import os
import os.path
import sys
import collections
from urllib.parse import urlparse, urljoin

# this is a hack to prevent matplotlib from trying to do interactive plot crap with e.g. Qt on
# remote machines.  See:
# https://stackoverflow.com/questions/21321292/using-matplotlib-when-display-is-undefined.
import matplotlib
matplotlib.use('Agg')

import matplotlib.pyplot as plt
import jinja2

MOOSE_DIR = os.environ.get('MOOSE_DIR', os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
os.environ['MOOSE_DIR'] = MOOSE_DIR
sys.path.append(os.path.join(MOOSE_DIR, 'python'))
os.environ['PYTHONPATH'] = os.environ.get('PYTHONPATH', '') + ':' + os.path.join(MOOSE_DIR, 'python')

from TestHarness.testers.bench import *
import hit

def find_moose_python():
    moosedir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    if 'MOOSE_DIR' in os.environ:
        moosedir = os.environ['MOOSE_DIR']
    moosepython = os.path.join(moosedir, 'python')

    if not os.path.exists(moosepython):
        raise Exception('Unable to locate moose/python directory, please set MOOSE_DIR environment variable')
    sys.path.append(moosepython)

find_moose_python()

def build_args():
    p = argparse.ArgumentParser()
    p.add_argument('--db', type=str, default=os.environ.get('MOOSE_SPEED_DB', 'speedtests.sqlite'), help='benchmark timings database')

    # options for comparing benchmarks
    p.add_argument('--old', type=str, default='', help='compare benchmark runs from the given revision (default prev of most recent)')
    p.add_argument('--new', type=str, default='', help='compare benchmark runs to the given revision (default most recent)')

    # options for running benchmarks
    p.add_argument('--run', action='store_true', help='run all benchmarks on the current checked out revision')
    p.add_argument('--perflog', action='store_true', help='store perflog benchmark info')
    p.add_argument('--rev', type=str, default='', help='manually specify git revision for this set of benchmarks')
    p.add_argument('--benchlist', type=str, default='bench.list', help='run all benchmarks on the current checked out revision')
    p.add_argument('--cum-dur', type=float, default=60, help='cumulative time (secs) to run each benchmark')
    p.add_argument('--min-runs', type=int, default=40, help='minimum number of runs for each benchmark')
    p.add_argument('--list-revs', action='store_true', help='list all benchmarked revisions in the db')
    p.add_argument('--trends', action='store_true', help='generate plots of historical trends of all benchmarks')
    p.add_argument('--psig', type=float, default=0.01, help='the p-value cutoffused to determine comparison significance')
    p.add_argument('--baseurl', type=str, default='https://github.com/idaholab/moose/commit/', help='the url prefix for commit links in generated visualization/output')
    return p

def main():
    p = build_args()
    args = p.parse_args()

    method = os.environ.get('METHOD', 'opt')

    if args.list_revs: # list all revisions for which there are benchmarks
        with DB(args.db) as db:
            revs, times = db.revisions(method=method)
            printed = set()
            for rev,t in zip(revs, times):
                if rev in printed:
                    continue
                printed.add(rev)
                tm = datetime.datetime.fromtimestamp(t)
                print('{}\t{}'.format(rev,tm))

    elif args.run: # run all benchmarks
        benches = read_benchmarks(args.benchlist)
        rootdir = os.path.dirname(args.benchlist)
        with DB(args.db) as db:
            for bench in benches:
                print('running "{}"...'.format(bench.name))
                t = Test(bench.executable, bench.infile, args=bench.args, rootdir=rootdir, perflog=bool(args.perflog))
                b = Bench(bench.name, test=t, cum_dur=args.cum_dur, min_runs=args.min_runs)
                b.run()
                if args.rev != '':
                    db.store(b, rev=args.rev)
                else:
                    db.store(b)

    elif args.trends: # generate print plots of benchmark runs over time
        with DB(args.db) as db:
            subdir = 'trends'
            if not os.path.exists(subdir):
                os.mkdir(subdir)
            benchnames = db.bench_names(method=method)
            buildpage(os.path.join(subdir, 'index.html'), benchnames, db, args.psig, method=method, baseurl=args.baseurl)
            for bname in db.bench_names(method=method):
                benches, revs = [], []
                dbrevs, _ = db.revisions(method=method)
                for rev in dbrevs:
                    try:
                        bench = db.load(rev, bname, method=method)
                        benches.append(bench)
                        revs.append(rev)
                    except:
                        pass
                if len(revs) > 25:
                    revs = revs[-25:]
                    benches = benches[-25:]
                plot(revs, benches, subdir=subdir, baseurl=args.baseurl)

    else: # compare two benchmarks
        with DB(args.db) as db:
            revs, _ = db.revisions(method=method)
            revold, revnew = revs[-min(len(revs), 2)], revs[-1]
            if args.old != '':
                revold = args.old
            if args.new != '':
                revnew = args.new

            # load benchmark objects for each revision+bench_name combo for
            cmps = compare(db, revold, revnew, args.psig, method=method)
            print(BenchComp.header(revold, revnew))
            for cmp in cmps:
                print(cmp)
            print(BenchComp.footer())

def buildpage(fname, plotnames, db, psig, lastn=60, method='opt', baseurl='https://github.com/idaholab/moose/commit/'):
    figpage = """
<!DOCTYPE html>
<meta charset="utf-8">
<html>
<head>
    <style>
        .flex-grid {
            display: flex;
            flex-wrap: wrap;
        }
        .center {
            text-align: center;
        }
        img.center {
            display: block;
            margin-left: auto;
            margin-right: auto;
        }
    </style>
</head>
<body>
    <div class="flex-grid">
        {% for name in benchnames %}
        <div>
            <object data="{{name}}.svg" type="image/svg+xml"> </object>
            <h2 class="center">{{name}}</h2>
        </div>
        {% endfor %}
    </div>

    <br><br><br>
    <div style="width:1700px; margin:auto auto; padding:50px;">
    <div style="width:800px; float:left; clear:left;">
        <h1>Progress Comparisons</h1>
        {% for c in comparisons %}
        <h3>Revision <a href="{{c.link1}}">{{c.revision1[:7]}}</a> to <a href="{{c.link2}}">{{c.revision2[:7]}}</a> ({{c.date1}} to {{c.date2}})</h3>
        <pre>{{c.text}}</pre>
        <br>
        {% endfor %}
    </div>

    <div style="width:800px; float:left">
        <h1>Reference Comparisons</h1>
        {% for c in refcomparisons %}
        <h3>Revision <a href="{{c.link1}}">{{c.revision1[:7]}}</a> to <a href="{{c.link2}}">{{c.revision2[:7]}}</a> ({{c.date1}} to {{c.date2}})</h3>
        <pre>{{c.text}}</pre>
        <br>
        {% endfor %}
    </div>
    </div>
</body>
</html>
"""

    revs, times = db.revisions(method=method)
    if len(revs) > lastn:
        revs = revs[-lastn:]
        times = times[-lastn:]

    tformat = '%Y/%m/%d-%H:%M'
    comparisons = []
    refcomparisons = []

    if len(revs) > 0:
        Comp = collections.namedtuple('Comp', 'revision1 revision2 date1 date2 link1 link2 text')
        for rev1, t1, rev2, t2 in zip(revs[:-1], times[:-1], revs[1:], times[1:]):
            s = BenchComp.header(rev1, rev2)
            cmps = compare(db, rev1, rev2, psig, method=method)
            for cmp in cmps:
                s += '\n' + cmp.__str__()
            s += '\n' + BenchComp.footer()

            t1 = datetime.datetime.fromtimestamp(t1).strftime(tformat)
            t2 = datetime.datetime.fromtimestamp(t2).strftime(tformat)
            link1 = urljoin(baseurl, rev1)
            link2 = urljoin(baseurl, rev2)
            c = Comp(rev1, rev2, t1, t2, link1, link2, s)
            comparisons.append(c)

        rev1, t1 = revs[0], datetime.datetime.fromtimestamp(times[0]).strftime(tformat)
        for rev2, t2 in zip(revs[1:], times[1:]):
            s = BenchComp.header(rev1, rev2)
            cmps = compare(db, rev1, rev2, psig, method=method)
            for cmp in cmps:
                s += '\n' + cmp.__str__()
            s += '\n' + BenchComp.footer()

            t2 = datetime.datetime.fromtimestamp(t2).strftime(tformat)
            link1 = urljoin(baseurl, rev1)
            link2 = urljoin(baseurl, rev2)
            c = Comp(rev1, rev2, t1, t2, link1, link2, s)
            refcomparisons.append(c)

    comparisons.reverse()
    refcomparisons.reverse()

    t = jinja2.Template(figpage)
    data = t.render(benchnames=plotnames, comparisons=comparisons, refcomparisons=refcomparisons)
    with open(fname, 'w') as f:
        f.write(data)

def compare(db, rev1, rev2, psig, method='opt'):
    # generate benchcomp comparison results
    bench_from = db.list(rev1, method=method)
    bench_to = db.list(rev2, method=method)

    old_times = collections.OrderedDict()
    new_times = collections.OrderedDict()
    for old, new in zip(bench_from, bench_to):
        old_times[old[1]] = db.load(rev1, old[1], method=method)
        new_times[new[1]] = db.load(rev2, new[1], method=method)

    cmps = []
    for name in old_times:
        if name not in new_times:
            continue

        old = old_times[name]
        new = new_times[name]
        if len(old.realruns) > 0 and len(new.realruns) > 0:
            cmp = BenchComp(old, new, psig=psig)
            cmps.append(cmp)
    return cmps

def plot(revisions, benchmarks, subdir='.', baseurl='https://github.com/idaholab/moose/commit/'):
    data = []
    labels = []
    for rev, bench in zip(revisions, benchmarks):
        data.append(bench.realruns)
        labels.append(rev[:7])

    median = sorted(data[0])[int(len(data[0])/2)]
    plt.axhline(y=median*1.05, linestyle='--', linewidth=2, color='red', alpha=.5, label='+5%')
    plt.axhline(y=median*1.01, linestyle=':', linewidth=2, color='red', label='+1%')
    plt.axhline(y=median, dashes=[48, 4, 12, 4], color='black', alpha=.5)
    plt.axhline(y=median*.99, linestyle=':', linewidth=2, color='green', label='-1%')
    plt.axhline(y=median*.95, linestyle='--', linewidth=2, color='green', alpha=.5, label='-5%')

    plt.boxplot(data, labels=labels, whis=1.5)
    plt.xticks(rotation=90)
    plt.ylabel('Time (seconds)')

    fig = plt.gcf()

    ax = fig.axes[0]
    labels = ax.get_xticklabels()
    for label in labels:
        label.set_url(urljoin(baseurl, label.get_text()))

    legend = ax.legend(loc='upper right')

    fig.subplots_adjust(bottom=.15)
    fig.savefig(os.path.join(subdir, benchmarks[0].name + '.svg'))
    plt.clf()
    #plt.show()

class BenchEntry:
    def __init__(self, name, executable, input_file, args):
        self.name = name
        self.executable = executable
        self.infile = input_file
        self.args = args.split(' ')

def read_benchmarks(benchlist):
    if not os.path.exists(benchlist):
        raise ValueError('benchmark list file "{}" does not exist'.format(benchlist))

    benches = []

    with open(benchlist, 'r') as f:
        data = f.read()

    root = hit.parse(benchlist, data)
    for child in root.find('benchmarks').children():
        if not child.param('binary'):
            raise ValueError('missing required "binary" field')
        ex = child.param('binary').strip()

        if not child.param('input'):
            raise ValueError('missing required "input" field')
        infile = child.param('input').strip()
        args = ''
        if child.find('cli_args'):
            args = child.param('cli_args')
        benches.append(BenchEntry(child.path(), ex, infile, args))
    return benches

if __name__ == '__main__':
    main()
