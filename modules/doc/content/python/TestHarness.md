# TestHarness

The TestHarness is the Python code behind the `run_tests` script in MOOSE and every MOOSE-based
application.  It is responsible for finding tests and running them.  Here we describe how to use
the `run_tests` script and how to create your own tests.

The ideas behind testing are described over the in the [MOOSE Test System](test_system.md)
documentation.

## run_tests

`run_tests` is a small Python script that can be found in every MOOSE-based application and in the
framework itself (under `moose/test`).  The `run_tests` script will find tests and run them with your
compiled binary for your app.

### Basic Usage

To run the tests for any application, make sure the executable is built (generally by running `make`
see the [Build System](build_system.md)) and then do:

```bash
./run_tests -j 8
```

There are many options for `run_tests`, but the `-j` option shown above is by far the most widely
used.  It tells the `run_tests` script how many processors to utilize on your local machine for
running tests concurrently.  Put the correct number there (instead of 8). The script will then go
find and run your tests and display the results.

### More Options id=moreoptions

To see more options for `run_tests` you can invoke it with `-h`.  There are many options to look
through, but some of the important ones are:

- `--failed-tests`: Runs the tests that just failed.  Note: as long as you keep using the
  `--failed-tests` option the set of failed tests will not change.
- `--n-threads #`: Causes the tests to run with `#` of (OpenMP/Pthread/TBB) threads.
- `-p #`: Causes the tests to run with `#` MPI processes.  Useful for guaranteeing that you get the
  same result in parallel!
- `--valgrind`: Run all of the tests with the [Valgrind](http://valgrind.org) utility.  Does
  extensive memory checking to catch segfaults and uninitialized memory errors.
- `--recover`: Run all of the tests in "recovery" mode.  This runs the test halfway, stops, then
  attempts to "recover" it and run until the end.  This is very rigorous testing that tests whether
  everything in your application can work with restart/recover.
- `--opt` (The default) Builds an optimized executable suitable for running calculations.
- `--dbg` An executable meant for use in a debugger (like gdb or lldb). Will be very slow and not meant to be used on large problems
- `--oprof` Not normally used. Can be used for "code profiling": running your code with a utility like oprof, gprof, vtune, etc.
- `--pro` An alias to `--oprof`
- `--devel` Something in-between opt and dbg. Attempts to be faster but also does some more run-time checks (asserts and mooseAsserts)

## `testroot`

The `testroot` file is a small configuration file that is formatted like a MOOSE input file.  It is
read by the `run_tests` script.  It should be placed in the root of your application directory
(i.e. right next to where the binary is).  Some things you can set in that file are:

- `app_name`: A unique, short name for your application.
- `allow_warnings`: `true` by default, set this to `false` to make all warnings from running tests be
  _errors_ instead.
- `allow_override` and `allow_unused`: `true` by default if set to `false` then syntax errors in your
  test input files will be treated as errors.

The one thing we do +not+ recommend is enforcing that the use of deprecated code should be treated
like an error.  That is entirely too rigid of a requirement and impedes the normal flow of
development.  Instead, developers should periodically run their tests with `--error-deprecated` to
see if any of their tests are using deprecated code / parameters and then fix them up at that point.
The MOOSE team is not responsible for fixing deprecated code.

## Influential Environment Variables id=environment

#### PYTHONPATH

PYTHONPATH instructs python to include the designated paths while attempting to import python modules. +While normally not needing to be set+, sometimes it is necessary. For example, when testing the TestHarness (unittests). Another use-case, is when a developer wants to utilize the moosedocs system for creating [moose documentation](MooseDocs/index.md) (the website you are using right now).

In either case, when you need to modify PYTHONPATH for MOOSE related development, you will almost always want to point it at `moose/python`.

PYTHONPATH functions just as PATH does (semi-colon separate list of paths, for which items contained within paths on the left, are found before items contained within paths on the right).

#### METHOD

Set the `METHOD` environment variable to one of the following to control which type of application binary to use:

| Variable Name | Argument | Usage |
| - | - | - |
| METHOD | opt | TestHarness will use the binary built with optimizations while running tests: `your_appname-opt` (the default) |
| METHOD | dbg | TestHarness will use the binary built with debugging symbols while running tests: `your_appname-dbg` |
| METHOD | oprof | TestHarness will use the binary built with code profiling while running tests: `your_appname-oprof` |
| METHOD | pro | An alias for oprof |
| METHOD | devel | Something in-between opt and dbg |

!alert note
The methods described here can also be controlled via command line arguments. See "More Options" above.

#### MOOSE_TERM_FORMAT

Set `MOOSE_TERM_FORMAT` to any or all of the following, as well as in a particular order and case (restricted) to control where, what, and how the TestHarness prints that specific item:

| Variable Name | Argument | Usage |
| - | - | - |
| MOOSE_TERM_FORMAT | c | Print caveats |
| MOOSE_TERM_FORMAT | j | Print justification filler |
| MOOSE_TERM_FORMAT | p | Print pre-formated status (10 character buffer fill) |
| MOOSE_TERM_FORMAT | s | Print status |
| MOOSE_TERM_FORMAT | n-N | Print test name |
| MOOSE_TERM_FORMAT | t | Print test completion time |

Example, if we set MOOSE_TERM_FORMAT to tpNsc, we would print the time, pre-fromatted status, test name (converted to upper-case), long naming status, and then the caveats. In that order:

```bash
MOOSE_TERM_FORMAT=tpNsc ./run_tests --re=simple_diffusion.test -p4 -t
[0.141s]       OK KERNELS/AD_SIMPLE_DIFFUSION.TEST [OVERSIZED]
[0.144s]       OK KERNELS/SIMPLE_DIFFUSION.TEST [OVERSIZED]
```

Caveats with the... caveats of MOOSE_TERM_FORMAT; When caveats are requested to be printed last, the TestHarness will allow the entire caveat to print, regardless of MOOSE_TERM_COLS (see below).

#### MOOSE_TERM_COLS

Set `MOOSE_TERM_COLS` to a positive integer, to set the available terminal column count to this amount:

| Variable Name | Argument | Usage |
| - | - | - |
| MOOSE_TERM_COLS | (int) | Allow for this many columns when printing output |

Example, if we set MOOSE_TERM_COLS to 50, we will restrict the default amount of columns the TestHarness normally uses while printing output:

```bash
MOOSE_TERM_COLS=50 ./run_tests --re=simple_diffusion.test
kernels/ad_simple_diffusion.test .............. OK
kernels/simple_diffusion.test ................. OK
--------------------------------------------------
Ran 2 tests in 2.9 seconds.
2 passed, 0 skipped, 0 pending, 0 failed
```

Caveats of MOOSE_TERM_COLS; If you specify to low a MOOSE_TERM_COLS, the TestHarness will only drop printing of the justification filler (see MOOSE_TERM_FORMAT above).
