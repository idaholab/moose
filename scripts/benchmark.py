#!/usr/bin/env python

import subprocess
import datetime
import time
import argparse
from scipy.stats import mannwhitneyu
import numpy
import sqlite3
import os
import sys
import shutil
import collections
import csv
import matplotlib.pyplot as plt
import jinja2
import pprint

def find_moose_python():
    moosedir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    if os.environ.has_key('MOOSE_DIR'):
        moosedir = os.environ['MOOSE_DIR']
    moosepython = os.path.join(moosedir, 'python')

    if not os.path.exists(moosepython):
        raise Exception('Unable to locate moose/python directory, please set MOOSE_DIR environment variable')
    sys.path.append(moosepython)

def build_args():
    p = argparse.ArgumentParser()
    p.add_argument('--db', type=str, default='benchmarks.sqlite', help='benchmark timings database')

    # options for comparing benchmarks
    p.add_argument('--old', type=str, default='', help='compare benchmark runs from the given revision (default prev of most recent)')
    p.add_argument('--new', type=str, default='', help='compare benchmark runs to the given revision (default most recent)')
    p.add_argument('--usertime', action='store_true', help='use user time instead of real time for comparisons')

    # options for running benchmarks
    p.add_argument('--run', action='store_true', help='run all benchmarks on the current checked out revision')
    p.add_argument('--rev', type=str, default='', help='manually specify git revision for this set of benchmarks')
    p.add_argument('--benchlist', type=str, default='bench.list', help='run all benchmarks on the current checked out revision')
    p.add_argument('--cum-dur', type=float, default=60, help='cumulative time (secs) to run each benchmark')
    p.add_argument('--min-runs', type=int, default=10, help='minimum number of runs for each benchmark')
    p.add_argument('--list-revs', action='store_true', help='list all benchmarked revisions in the db')
    p.add_argument('--trends', action='store_true', help='generate plots of historical trends of all benchmarks')
    return p

def main():
    p = build_args()
    args = p.parse_args()

    if args.list_revs: # list all revisions for which there are benchmarks
        with BenchmarkDB(args.db) as db:
            revs, times = db.revisions()
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
        with BenchmarkDB(args.db) as db:
            for bench in benches:
                print('running "{}"...'.format(bench.name))
                t = Test(bench.executable, bench.infile, args=bench.args, rootdir=rootdir)
                b = Benchmark(bench.name, test=t, cum_dur=args.cum_dur, min_runs=args.min_runs)
                b.run()
                if args.rev != '':
                    db.store(b, rev=args.rev)
                else:
                    db.store(b)

    elif args.trends: # generate print plots of benchmark runs over time
        with BenchmarkDB(args.db) as db:
            benchlist = read_benchmarks(args.benchlist)
            benchnames = []
            for bb in benchlist:
                benchnames.append(bb.name)

            subdir = 'trends'
            if not os.path.exists(subdir):
                os.mkdir(subdir)
            buildpage(os.path.join(subdir, 'index.html'), benchnames)
            for bname in benchnames:
                benches = []
                revs, _ = db.revisions()
                for rev in revs:
                    benches.append(db.load(rev, bname))
                plot(revs, benches, subdir=subdir)

    else: # compare two benchmarks
        with BenchmarkDB(args.db) as db:
            (revs, _) = db.revisions()
            revold, revnew = revs[-2], revs[-1]
            if args.old != '':
                revold = args.old
            if args.new != '':
                revnew = args.new

            # load benchmark objects for each revision+bench_name combo for
            bench_from = db.list(revold)
            bench_to = db.list(revnew)
            old_times = collections.OrderedDict()
            new_times = collections.OrderedDict()
            for old, new in zip(bench_from, bench_to):
                old_times[old[1]] = db.load(old[0], old[1])
                new_times[new[1]] = db.load(new[0], new[1])

            print(BenchComp.header(revold, revnew))
            for name in old_times:
                if name not in new_times:
                    continue

                # print before-and-after comparison for each benchmark
                old = old_times[name]
                new = new_times[name]
                if len(old.realruns) > 0 and len(new.realruns) > 0:
                    cmp = BenchComp(old, new, usertime=args.usertime)
                    print(cmp)
            print(BenchComp.footer())

def buildpage(fname, plotnames):
    figpage = """
<!DOCTYPE html>
<meta charset="utf-8">
<html>
<head>
    <style>
        .flex-grid {
            /*margin: 0 auto;*/
            /*float: left;*/
            /*width: 25%;*/
            display: flex;
        }
        .col {
            flex: 1;
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
        <div class="col">
            <img class="center" src="{{name}}.svg">
            <h3 class="center">{{name}}</h3>
        </div>
        {% endfor %}
    </div>
</body>
</html>
"""

    t = jinja2.Template(figpage)
    data = t.render(benchnames=plotnames)
    with open(fname, 'w') as f:
        f.write(data)

def plot(revisions, benchmarks, subdir='.'):
    data = []
    labels = []
    for rev, bench in zip(revisions, benchmarks):
        data.append(bench.realruns)
        labels.append(rev[:6])

    plt.boxplot(data, labels=labels)
    plt.xticks(rotation=90)

    fig = plt.gcf()
    fig.subplots_adjust(bottom=.15)
    fig.savefig(os.path.join(subdir, benchmarks[0].name + '.svg'))
    plt.clf()
    #plt.show()

class Test:
    def __init__(self, executable, infile, rootdir='.', args=None):
        self.rootdir = rootdir
        self.executable = executable
        self.infile = infile
        self.args = args
        self.dur_secs = 0
        self.user_dur_secs = 0
        self.perflog = []

    def run(self):
        cmdpath = os.path.abspath(os.path.join(self.rootdir, self.executable))
        infilepath = os.path.abspath(os.path.join(self.rootdir, self.infile))
        cmd = [cmdpath, '-i', infilepath]
        if self.args is not None:
            cmd.extend(self.args)
        cmd.append('Outputs/console=false')
        cmd.append('UserObjects/perflog/type=PerfLogDumper')

        tmpdir = 'tmp'
        shutil.rmtree(tmpdir, ignore_errors=True)
        os.makedirs(tmpdir)

        with open(os.devnull, 'w') as devnull:
            p = subprocess.Popen(cmd, cwd=tmpdir, stdout=devnull)
            start = time.time()
            (_, retcode, ru) = os.wait4(p.pid, 0) # do this hack to retrieve the usertime of the subprocess
            end = time.time()

        if retcode != 0:
            raise RuntimeError('command {} returned nonzero exit code'.format(cmd))

        self.dur_secs = end - start
        self.user_dur_secs = ru.ru_utime

        # write perflog
        with open(os.path.join(tmpdir, 'perflog.csv'), 'r') as csvfile:
            reader = csv.reader(csvfile)
            skip = True # use to skip header line
            for row in reader:
                if not skip:
                    self.perflog.append(row)
                else:
                    skip = False

        shutil.rmtree('tmp')


class Benchmark:
    def __init__(self, name, realruns=None, userruns=None, test=None, cum_dur=60, min_runs=10, max_runs=1000):
        self.name = name
        self.test = test
        self.realruns = []
        self.userruns = []
        self.perflogruns = []
        if realruns is not None:
            self.realruns.extend(realruns)
        if userruns is not None:
            self.userruns.extend(userruns)
        self._cum_dur = cum_dur
        self._min_runs = min_runs
        self._max_runs = max_runs

    def run(self):
        # TODO: also collect MOOSE perflog data
        tot = 0.0
        while True:
            self.test.run()
            self.realruns.append(self.test.dur_secs)
            self.userruns.append(self.test.user_dur_secs)
            self.perflogruns.append(self.test.perflog)
            tot += self.test.dur_secs
            if (len(self.realruns) >= self._min_runs and tot >= self._cum_dur) or len(self.realruns) >= self._max_runs:
                break

class BenchComp:
    def __init__(self, oldbench, newbench, psig=0.05, usertime=False):
        self.name = oldbench.name
        self.psig = psig
        self.old = oldbench.realruns
        self.new = newbench.realruns
        if usertime:
            self.old = oldbench.userruns
            self.new = newbench.userruns

        self.iqr_old = _iqr(self.old)
        self.iqr_new = _iqr(self.new)
        try:
            result = mannwhitneyu(self.iqr_old, self.iqr_new, alternative='two-sided')
            self.pvalue = result.pvalue
        except:
            self.pvalue = 1.0

        self.u = result[0]
        self.avg_old = float(sum(self.iqr_old))/len(self.iqr_old)
        self.avg_new = float(sum(self.iqr_new))/len(self.iqr_new)
        self.speed_change = (self.avg_new - self.avg_old) / self.avg_old

    @classmethod
    def header(cls, revold, revnew):
        oldstr, newstr = revold, revnew
        if len(oldstr) > 12:
            oldstr = oldstr[:12]
        if len(newstr) > 12:
            newstr = newstr[:12]
        revstr = ' {} to {} '.format(oldstr, newstr)
        revstr = revstr.center(30,'-')
        return '' \
            + '--------------------------------{}--------------------------------'.format(revstr) \
            + '\n{:^30s}   {:^15s}   {:^15s}   {:5s}'.format('benchmark', 'old (sec/run)', 'new (sec/run)', 'speedup (pvalue,nsamples)') \
            + '\n----------------------------------------------------------------------------------------------'
    @classmethod
    def footer(cls):
        return '----------------------------------------------------------------------------------------------'

    def __str__(self):
        name = self.name
        if len(name) > 30:
            name = name[:27] + '...'
        if self.pvalue <= self.psig:
            return '{:>30s}:   {:^15f}   {:^15f}   {:+5.1f}% (p={:.3f} n={}+{})'.format(name, self.avg_old, self.avg_new, self.speed_change*100, self.pvalue, len(self.iqr_old), len(self.iqr_new))
        else:
            return '{:>30s}:   {:^15f}   {:^15f}      ~   (p={:.3f} n={}+{})'.format(name, self.avg_old, self.avg_new, self.pvalue, len(self.iqr_old), len(self.iqr_new))

def _iqr(a):
    """return elements of a and be in the interquartile range (inclusive) of both a and b"""
    qup, qlow = numpy.percentile(a, [75 ,25])

    iqra = []
    for val in a:
        if qlow <= val and val <= qup:
            iqra.append(val)
    return iqra


class BenchmarkDB:
    def __init__(self, fname):
        CREATE_BENCH_TABLE = '''CREATE TABLE IF NOT EXISTS benchmarks
        (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          name TEXT,
          executable TEXT,
          input_file TEXT,
          timestamp INTEGER,
          revision TEXT,
          date INTEGER,
          load REAL
        );'''

        CREATE_TIMES_TABLE = '''CREATE TABLE IF NOT EXISTS timings
        (
          benchmark_id INTEGER,
          run INTEGER,
          realtime_secs REAL,
          usertime_secs REAL
        );'''

        CREATE_PERFLOG_TABLE = '''CREATE TABLE IF NOT EXISTS perflog
        (
          benchmark_id INTEGER,
          run INTEGER,
          field TEXT,
          subfield TEXT,
          exec_count INTEGER,
          self_time_secs REAL,
          cum_time_secs REAL
        );'''

        self.fname = fname
        self.conn = sqlite3.connect(fname)
        c = self.conn.cursor()
        c.execute(CREATE_BENCH_TABLE)
        c.execute(CREATE_TIMES_TABLE)
        c.execute(CREATE_PERFLOG_TABLE)

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()

    def revisions(self):
        c = self.conn.cursor()
        c.execute('SELECT DISTINCT revision,date FROM benchmarks ORDER BY date ASC')
        rows = c.fetchall()
        revs = []
        times = []
        for r in rows:
            revs.append(r[0])
            times.append(r[1])
        return revs, times

    def list(self, revision, benchmark=''):
        c = self.conn.cursor()
        if benchmark == '':
            c.execute('SELECT id,name,executable,input_file FROM benchmarks WHERE INSTR(revision,?) ORDER BY date ASC', (revision,))
        else:
            c.execute('SELECT id,name,executable,input_file FROM benchmarks WHERE INSTR(revision,?) AND INSTR(name,?) ORDER BY date ASC', (revision,benchmark))
        benchmarks = c.fetchall()
        return benchmarks

    def load_times(self, bench_id):
        c = self.conn.cursor()
        c.execute('SELECT realtime_secs,usertime_secs FROM timings WHERE benchmark_id=?', (bench_id,))
        ents = c.fetchall()
        real, user = [], []
        for ent in ents:
            real.append(float(ent[0]))
            user.append(float(ent[1]))
        return real, user

    def load(self, revision, bench_name):
        """loads and returns a Benchmark object for the given revision and benchmark name"""
        entries = self.list(revision, benchmark=bench_name)
        b = entries[0]
        real, user = self.load_times(b[0])
        return Benchmark(b[1], test=Test(b[2], b[3]), realruns=real, userruns=user)

    def store(self, benchmark, rev=None):
        """stores a (run/executed) Benchmark in the database. if rev is None, git revision is retrieved from git"""
        ex = benchmark.test.executable
        infile = benchmark.test.infile
        timestamp = time.time()
        date = timestamp
        if rev is None:
            rev, date = git_revision()
        load = os.getloadavg()[0]

        c = self.conn.cursor()
        c.execute('INSERT INTO benchmarks (name,executable,input_file,timestamp,revision,date,load) VALUES (?,?,?,?,?,?,?)',
                (benchmark.name, ex, infile, timestamp, rev, date, load))
        self.conn.commit()

        c.execute('SELECT max(id) from benchmarks')
        bench_id = int(c.fetchone()[0])

        i = 0
        for real, user, perflog in zip(benchmark.realruns, benchmark.userruns, benchmark.perflogruns):
            c.execute('INSERT INTO timings (benchmark_id, run, realtime_secs, usertime_secs) VALUES (?,?,?,?)', (bench_id, i, real, user))
            i += 1
            for entry in perflog:
                cat, subcat, nruns, selftime, cumtime = entry
                c.execute('INSERT INTO perflog (benchmark_id, run, field, subfield, exec_count, self_time_secs, cum_time_secs) VALUES (?,?,?,?,?,?,?)',
                        (bench_id, i, cat, subcat, nruns, selftime, cumtime))

        return bench_id

    def close(self):
        self.conn.commit()
        self.conn.close()

def git_revision(dir='.'):
    # return hash and (unix secs since epoch) date
    cmd = ['git', 'log', '--date', 'unix', '--pretty=format:%H %ad', '-n', '1']
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, cwd=dir)
    stdout, stderr = p.communicate()
    if p.returncode != 0:
        raise RuntimeError('failed to retrieve git revision')
    commit = stdout.strip().split(' ')[0]
    date = int(stdout.strip().split(' ')[1])
    return commit, date

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
    root = readInputFile(benchlist).children['benchmarks']
    for benchname in root.children_list:
        child = root.children[benchname]
        ex = child.params['binary'].strip()
        infile = child.params['infile'].strip()
        args = ''
        if 'args' in child.params:
            args = child.params['args']
        benches.append(BenchEntry(benchname, ex, infile, args))
    return benches

if __name__ == '__main__':
    find_moose_python()
    from FactorySystem.ParseGetPot import readInputFile
    main()

