# Performance Benchmarking

Utilities for doing performance benchmarking of MOOSE-based applications are included in the main
MOOSE repository.  These utilities provide functionality for benchmarking and tracking MOOSE
performance.  They can be used to run benchmarks, generate trend visualizations, and look at stats
comparing bencharks between various revisions.  The following sections describe how to setup a
benchmark machine and use it to run benchmarks and visualize results.

## Tuning a Benchmarking Machine

In order to obtain accurate results, you need to run the benchmark process(es)
as close to isolated as possible.  On a linux system, you should e.g. use cpu
isolation via setting kernel boot parameters:

```text
isolcpus=[n] rcu_nocbs=[n]
```

in your boot loader (e.g. grub).  The benchmarking tools/scripts in MOOSE should automatically
detect CPU isolation on Linux and schedule benchmark jobs to those CPUs. You should also disable
any turbo functionality.  For example on `intel_pstate` driver cpus:

```text
$ echo "1" > /sys/devices/system/cpu/intel_pstate/no_turbo
```

You will also want to turn off any hyperthreading for cores you use for benchmarking.  You can do
this in the bios or by something like:

```text
$ echo "0" > /sys/devices/system/cpu/cpu[n]/online
```

for each hyperthread core you want running - you can look in `/proc/cpuinfo` for pairs of cpus
that have the same core id turning off one of the pair.  These will need to be done on every boot.
You can use the sysfsutils package and its `/etc/sysfs.conf` configuration file to do this
persistently on boot - i.e.:

```text
devices/system/cpu/intel_pstate/no_turbo = 1
devices/system/cpu/cpu3/online = 0
devices/system/cpu/cpu5/online = 0
```

## Test Harness Benchmarks

Benchmarks can be run through the test harness (i.e.  using the `run_tests` script) by doing
e.g. `./run_tests --run speedtests`.  When this is done, the test harness looks for test spec
files named `speedtests` just like the `tests` files that contain regular moose test details.
The format for these files is:

```text
[Benchmarks]
    [benchmark-name]
        type = SpeedTest
        input = input-file-name.i
        cli_args = '--an-arg=1 a/hit/format/cli/arg=foo'
        # optional:
        min_runs = 15 # default 40
        max_runs = 100 # default 400
        cumulative_dur = 100 # default 60 sec
    []

    [./benchmark2-name]
        type = SpeedTest
        input = another-input-file-name.i
        cli_args = 'some/cli/arg=bar'
    []

    # ...
[]
```

After being run, benchmark data are stored in a sqlite database (default name
`speedtests.sqlite`).  When the test harness is run without the `--run speedtests` flag, tests
described in `speedtests` files are run in *check-only* mode where moose just checks that their
input files are well-formed and parse correctly without actually running them.


## Manual/Direct Benchmarks

The `[moose-repo]/scripts/benchmark.py` script can be used to manually list and directly run benchmarks without the
test harness (for hacking, debugging, etc.).  To do this, the script reads a `bench.list` text
file that specifies which input files should be run and corresponding (benchmark) names for them
along with any optional arguments.  The `bench.list` file has the following format:

```text
[benchmarks]
    [./simple_diffusion_refine3]
        binary = test/moose_test-opt
        input = test/tests/kernels/simple_diffusion/simple_diffusion.i
        cli_args = 'Mesh/uniform_refine=3'
    [../]
    [./simple_diffusion_refine4]
        binary = test/moose_test-opt
        input = test/tests/kernels/simple_diffusion/simple_diffusion.i
        cli_args = 'Mesh/uniform_refine=4'
    [../]
    [./simple_diffusion_ref5]
        binary = test/moose_test-opt
        input = test/tests/kernels/simple_diffusion/simple_diffusion.i
        cli_args = 'Mesh/uniform_refine=5'
    [../]
    # ... add as many as you want
[]
```

To run the manual benchmarks directly, do this:

```text
$ ./scripts/benchmark.py --run
```

When benchmarks are run, the binaries specified in `bench.list` must already exist.  Benchmark
data are then stored in a sqlite database (default name `speedtests.sqlite`).  You can specify
the minimum number of runs for each benchmark problem/simulation with the `--min-runs` (default
10).  Each benchmark will be run as many times as possible within 1 minute (customizable via the
`--cum-dur` flag) or the specified minimum number of times (whichever is larger). 

## Analyzing Results

Regardless of how you ran the benchmarks (either by this script or using the test harness), MOOSE
revisions with available benchmark data can be listed (from the database) by running:

```text
$ ./benchmark.py --list-revs
44d2f3434b3346dc14fc9e86aa99ec433c1bbf10	2016-09-07 19:36:16
86ced0d0c959c9bdc59497f0bc9324c5cdcd7e8f	2016-09-08 09:29:17
447b455f1e2d8eda649468ed03ef792504d4b467	2016-09-08 09:43:56
...
```

To look at stats comparing benchmark data from two revisions, run:

```text
$ ./benchmark.py # defaults to using the most recent two revisions of benchmark data
-------------------------------- 871c98630c98 to 38bb6f5ebe5f --------------------------------
          benchmark               old (sec/run)     new (sec/run)    speedup (pvalue,nsamples)
----------------------------------------------------------------------------------------------
    simple diffusion (refine3):      0.408034          0.408034          ~   (p=0.996 n=36+36)

    simple diffusion (refine4):      1.554724          1.561682          ~   (p=0.571 n=10+10)
    simple diffusion (refine5):      6.592326          6.592326          ~   (p=0.882 n=4+4)
----------------------------------------------------------------------------------------------

$ ./benchmark.py -old 44d2f34 -new 447b455 # or specify revisions to compare manually
------------------------------------- 44d2f34 to 447b455 -------------------------------------
          benchmark               old (sec/run)     new (sec/run)    speedup (pvalue,nsamples)
----------------------------------------------------------------------------------------------
    simple diffusion (refine3):      0.416574          0.411435        -1.2% (p=0.000 n=37+37)
    simple diffusion (refine4):      1.554724          1.497379        -3.7% (p=0.000 n=10+11)
    simple diffusion (refine5):      6.553244          6.360004        -2.9% (p=0.030 n=4+4)
----------------------------------------------------------------------------------------------
```

To generate visualizations, run:

```text
$ ./scripts/benchmark.py --trends
```

This will generate an svg box plot for each benchmark over time/revision in a `trends`
subdirectory.  An `index.html` file is also generated that embeds all the svg plots for
convenient viewing all together in a browser.

