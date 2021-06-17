#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess
import time
import sys
import os
import gc
import shutil
import csv
import tempfile
import threading

# try to import the resource module. We check further down if it failed
try:
    import resource
except:
    pass

from TestHarness.testers.Tester import Tester

def process_timeout(proc, timeout_sec):
  kill_proc = lambda p: p.kill()
  timer = threading.Timer(timeout_sec, kill_proc, [proc])
  try:
    timer.start()
    proc.wait()
  finally:
    timer.cancel()

class Test:
    def __init__(self, executable, infile, rootdir='.', args=None, perflog=False):
        self.rootdir = rootdir
        self.executable = executable
        self.infile = infile
        self.args = args
        self.dur_secs = 0
        self.perflog = []
        self.getpot_options = ['Outputs/console=false', 'Outputs/exodus=false', 'Outputs/csv=false', '--no-gdb-backtrace']
        self.have_perflog = perflog
        if self.have_perflog:
            self.getpot_options.append('UserObjects/perflog/type=PerflogDumper')

    def _buildcmd(self):
        cmdpath = self.executable
        infilepath = os.path.abspath(os.path.join(self.rootdir, self.infile))
        cmd = [cmdpath, '-i', infilepath]
        if self.args is not None:
            cmd.extend(self.args)
        cmd.extend(self.getpot_options)

        # check for linux cpu isolation
        isolpath = '/sys/devices/system/cpu/isolated'
        cpuid = None
        if os.path.exists(isolpath):
            with open(isolpath, 'r') as f:
                cpus = f.read().split(',')
                if len(cpus[0].strip()) > 0:
                    cpuid = cpus[0]
        if cpuid:
            cmd = ['taskset', '-c', cpuid] + cmd
        return cmd

    def reset(self):
        self.perflog = []
        self.dur_secs = 0

    def run(self, timer=None, timeout=300):
        self.reset()
        cmd = self._buildcmd()

        tmpdir =  tempfile.mkdtemp()
        shutil.rmtree(tmpdir, ignore_errors=True)
        os.makedirs(tmpdir)

        rusage = resource.getrusage(resource.RUSAGE_CHILDREN)
        start = rusage.ru_utime
        gc.disable()
        with open(os.devnull, 'w') as devnull:
            if timer:
                timer.start()
            p = subprocess.Popen(cmd, cwd=tmpdir, stdout=devnull, stderr=devnull)
            process_timeout(p, timeout)
            if timer:
                timer.stop()
        gc.enable()
        rusage = resource.getrusage(resource.RUSAGE_CHILDREN)
        end = rusage.ru_utime

        if p.returncode != 0:
            raise RuntimeError('command {} returned nonzero exit code'.format(cmd))

        self.dur_secs = end - start

        # write perflog
        if self.have_perflog:
            with open(os.path.join(tmpdir, 'perflog.csv'), 'r') as csvfile:
                reader = csv.reader(csvfile)
                skip = True # use to skip header line
                for row in reader:
                    if not skip:
                        self.perflog.append(row)
                    else:
                        skip = False

        shutil.rmtree(tmpdir)

class SpeedTest(Tester):
    @staticmethod
    def validParams():
        params = Tester.validParams()
        params.addParam('input',              'The input file to use for this test.')
        params.addParam('test_name',          'The name of the test - populated automatically')
        params.addParam('cumulative_dur', 60, 'cumulative time (secs) to run each benchmark')
        params.addParam('min_runs', 40,       'minimum number of runs for each benchmark')
        params.addParam('max_runs', 400,      'maximum number of runs for each benchmark')
        params.addParam('perflog', False,     'true to enable perflog and store its output')
        return params

    def __init__(self, name, params):
        Tester.__init__(self, name, params)
        self.tags.append('speedtests')
        self.timeout = max(3600, float(params['max_time']))
        self.check_only = False

        self.params = params
        self.benchmark = None
        self.db = os.environ.get('MOOSE_SPEED_DB', 'speedtests.sqlite')

    # override
    def getMaxTime(self):
        return self.timeout

    # override
    def checkRunnable(self, options):
        # check if resource is available
        if 'resource' not in sys.modules:
            return False

        # if user is not explicitly running benchmarks, we only run moose once and just check
        # input - to make sure the benchmark isn't broken.
        if 'speedtests' not in options.runtags:
            self.params['max_runs'] = 1
            self.params['cli_args'].insert(0, '--check-input')
            self.check_only = True
        return True

    # override
    def run(self, timer, options):
        p = self.params
        if not self.check_only and options.method not in ['opt', 'oprof', 'dbg']:
            raise ValueError('cannot run benchmark with "' + options.method + '" build')
        t = Test(p['executable'], p['input'], args=p['cli_args'], rootdir=p['test_dir'], perflog=p['perflog'])

        if self.check_only:
            t.run(timer, timeout=p['max_time'])
            return

        name = p['test_name'].split('.')[-1]
        self.benchmark = Bench(name, test=t, cum_dur=float(p['cumulative_dur']), min_runs=int(p['min_runs']), max_runs=int(p['max_runs']))
        self.benchmark.run(timer, timeout=self.timeout)
        with DB(self.db) as db:
            db.store(self.benchmark)

    # override
    def processResults(self, moose_dir, options, output):
        self.setStatus(self.success)
        return output

class Bench:
    def __init__(self, name, realruns=None, test=None, cum_dur=60, min_runs=40, max_runs=400):
        self.name = name
        self.test = test
        self.realruns = []
        self.perflogruns = []
        if realruns is not None:
            self.realruns.extend(realruns)
        self._cum_dur = cum_dur
        self._min_runs = min_runs
        self._max_runs = max_runs

    def run(self, timer=None, timeout=3600):
        tot = 0.0
        start = time.time()
        while (len(self.realruns) < self._min_runs or tot < self._cum_dur) and len(self.realruns) < self._max_runs:
            dt = time.time() - start
            if dt >= timeout:
                raise RuntimeError('benchmark timed out after {} with {} runs'.format(dt, len(self.realruns)))

            self.test.run(timer, timeout=timeout - dt)
            self.realruns.append(self.test.dur_secs)
            self.perflogruns.append(self.test.perflog)
            tot += self.test.dur_secs

class BenchComp:
    def __init__(self, oldbench, newbench, psig=0.01):
        self.name = oldbench.name
        self.psig = psig
        self.old = oldbench.realruns
        self.new = newbench.realruns

        self.iqr_old = _iqr(self.old)
        self.iqr_new = _iqr(self.new)

        from scipy.stats import mannwhitneyu
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
            + '\n{:^30s}   {:^15s}   {:^15s}   {:5s}'.format('benchmark', 'old (sec/run)', 'new (sec/run)', 'speedup (pvalue, nsamples)') \
            + '\n----------------------------------------------------------------------------------------------'
    @classmethod
    def footer(cls):
        return '----------------------------------------------------------------------------------------------'

    def __str__(self):
        name = self.name
        if len(name) > 30:
            name = name[:27] + '...'
        if self.pvalue <= self.psig:
            return '{:>30s}:   {:^15f}   {:^15f}   {:+5.1f}% (p={:.4f},n={}+{})'.format(name, self.avg_old, self.avg_new, self.speed_change*100, self.pvalue, len(self.iqr_old), len(self.iqr_new))
        else:
            return '{:>30s}:   {:^15f}   {:^15f}      ~   (p={:.4f},n={}+{})'.format(name, self.avg_old, self.avg_new, self.pvalue, len(self.iqr_old), len(self.iqr_new))

def _iqr(a, frac=1000):
    """return elements of a within frac*iqr of the the interquartile range (inclusive)"""
    import numpy
    qup, qlow = numpy.percentile(a, [75 ,25])

    iqr = qup - qlow
    clean = []
    for val in a:
        if qlow - frac*iqr <= val and val <= qup + frac*iqr:
            clean.append(val)
    return clean

class DB:
    def __init__(self, fname):
        CREATE_BENCH_TABLE = '''CREATE TABLE IF NOT EXISTS benchmarks
        (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          name TEXT,
          executable TEXT,
          executable_name TEXT,
          executable_method TEXT,
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
          realtime_secs REAL
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

        # python might not have sqlite3 builtin, so do the import here so
        # that the TestHarness can always import this file
        import sqlite3
        self.conn = sqlite3.connect(fname)
        c = self.conn.cursor()
        c.execute(CREATE_BENCH_TABLE)
        c.execute(CREATE_TIMES_TABLE)
        c.execute(CREATE_PERFLOG_TABLE)

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()

    def revisions(self, method='opt'):
        c = self.conn.cursor()
        c.execute('SELECT revision,date FROM benchmarks WHERE executable_method=? GROUP BY revision ORDER BY date ASC', (method,))
        rows = c.fetchall()
        revs = []
        times = []
        for r in rows:
            revs.append(r[0])
            times.append(r[1])
        return revs, times

    def bench_names(self, method='opt'):
        c = self.conn.cursor()
        c.execute('SELECT DISTINCT name FROM benchmarks WHERE executable_method=?', (method,))
        rows = c.fetchall()
        names = []
        for r in rows:
            names.append(r[0])
        return names

    def list(self, revision, benchmark='', method='opt'):
        c = self.conn.cursor()
        if benchmark == '':
            c.execute('SELECT id,name,executable,input_file FROM benchmarks WHERE INSTR(revision,?) AND executable_method=? ORDER BY date ASC', (revision,method))
        else:
            c.execute('SELECT id,name,executable,input_file FROM benchmarks WHERE INSTR(revision,?) AND name=? AND executable_method=? ORDER BY date ASC', (revision,benchmark,method))
        benchmarks = c.fetchall()
        return benchmarks

    def load_times(self, bench_id):
        c = self.conn.cursor()
        c.execute('SELECT realtime_secs FROM timings WHERE benchmark_id=?', (bench_id,))
        ents = c.fetchall()
        real = []
        for ent in ents:
            real.append(float(ent[0]))
        return real

    def load(self, revision, bench_name, method='opt'):
        """loads and returns a Bench object for the given revision and benchmark name"""
        entries = self.list(revision, benchmark=bench_name, method=method)
        if len(entries) < 1:
            raise RuntimeError('load: no benchamrk for revision="{}",bench_name="{}"'.format(revision, bench_name))
        b = entries[0]
        real = self.load_times(b[0])
        return Bench(b[1], test=Test(b[2], b[3]), realruns=real)

    def store(self, benchmark, rev=None):
        """stores a (run/executed) Bench in the database. if rev is None, git revision is retrieved from git"""
        ex = benchmark.test.executable
        (ex_name, ex_method) = os.path.basename(ex).rsplit('-', 1)
        infile = benchmark.test.infile
        timestamp = time.time()
        date = timestamp
        if rev is None:
            if 'MOOSE_REVISION' in os.environ:
                rev = os.environ['MOOSE_REVISION']
            else:
                rev, date = git_revision()
        load = os.getloadavg()[0]

        c = self.conn.cursor()
        c.execute('INSERT INTO benchmarks (name,executable,executable_name,executable_method,input_file,timestamp,revision,date,load) VALUES (?,?,?,?,?,?,?,?,?)',
                (benchmark.name, ex, ex_name, ex_method, infile, timestamp, rev, date, load))
        bench_id = c.lastrowid
        self.conn.commit()

        i = 0
        for real, perflog in zip(benchmark.realruns, benchmark.perflogruns):
            c.execute('INSERT INTO timings (benchmark_id, run, realtime_secs) VALUES (?,?,?)', (bench_id, i, real))
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
    cmd = ['git', 'log', '--date', 'raw', '--pretty=format:%H %ad', '-n', '1']
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, cwd=dir)
    stdout, stderr = p.communicate()
    if p.returncode != 0:
        raise RuntimeError('failed to retrieve git revision')
    commit = str(stdout).strip().split(' ')[0]
    date = int(str(stdout).strip().split(' ')[1])
    return commit, date
