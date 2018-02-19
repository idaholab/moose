# TestHarness

The TestHarness is the Python code behind the `run_tests` script in MOOSE and every MOOSE-based application.  It is responsible for finding tests and running them.  Here we'll describe how to use the `run_tests` script and how to create your own tests.

The ideas behind testing are described over the in the [MOOSE Test System](test_system.md) documentation.

## run_tests
`run_tests` is a small Python script that can be found in every MOOSE-based application and in the framework itself (under `moose/test`).  The `run_tests` script will find tests and run them with your compiled binary for your app.

### Basic Usage

To run the tests for any application, make sure the executable is built (generally by running `make` see the [Build System](build_system.md)) and then do:

```bash
./run_tests -j 8
```

There are many options for `run_tests`, but the `-j` option shown above is by far the most widely used.  It tells the `run_tests` script how many processors to utilize on your local machine for running tests concurrently.  Put the correct number there (instead of 8).

The script will then go find and run your tests and display the results


### More Options

To see more options for `run_tests` you can invoke it with `-h`.  There are many options to look through, but some of the important ones are:

 * `--failed-tests`: Runs the tests that just failed.  Note: as long as you keep using the `--failed-tests` option the set of failed tests will not change.
 * `--n-threads #`: Causes the tests to run with `#` of (OpenMP/Pthread/TBB) threads.
 * `-p #`: Causes the tests to run with `#` MPI processes.  Useful for guaranteeing that you get the same result in parallel!
 * `--valgrind`: Run all of the tests with the [Valgrind](http://valgrind.org) utility.  Does extensive memory checking to catch segfaults and uninitialized memory errors.
 * `--recover`: Run all of the tests in "recovery" mode.  This runs the test halfway, stops, then attempts to "recover" it and run until the end.  This is very rigorous testing that tests whether everything in your application can work with restart/recover.

## `testroot`

The `testroot` file is a small configuration file that is formatted like a MOOSE input file.  It is read by the `run_tests` script.  It should be placed in the root of your application directory (i.e. right next to where the binary is).  Some things you can set in that file are:

 * `app_name`: A unique, short name for your application
 * `allow_warnings`: `true` by default, set this to `false` to make all warnings from running tests be _errors_ instead.
 * `allow_override` and `allow_unused`: `true` by default if set to `false` then syntax errors in your test input files will be treated as errors.

The one thing we do _NOT_ recommend is enforcing that the use of deprecated code should be treated like an error.  That is entirely too rigid of a requirement and impedes the normal flow of development.  Instead, developers should periodically run their tests with `--error-deprecated` to see if any of their tests are using deprecated code / parameters and then fix them up at that point.  The MOOSE team is not responsible for fixing deprecations.