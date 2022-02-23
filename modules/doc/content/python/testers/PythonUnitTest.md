# PythonUnitTest

The `PythonUnitTest` tester is used to run Python scripts in the MOOSE test suite.
These are usually unit tests, but can also be running a [Method of Manufactured solutions](python/mms.md)
study. The test passes if the python scripts returns normally, and fails if an error/exception is met.

!alert note
Recover testing and valgrind memory-checking is disabled by default for the `PythonUnitTest`.

## Options

Test configuration options are specified in the `tests` file.

- `input`: The python input file to use for this test

- `test_case`: The specific test case to run. Defaults to all test cases in the module

- `buffer`: Equivalent to passing `-b` or `--buffer` to the unittest. Defaults to False

- `separate`: Run each test in the file in a separate subprocess. Defaults to False


Other test commands & restrictions may be found in the [TestHarness documentation](TestHarness.md).

## Example test configuration in the MOOSE test suite

In this example, `PythonUnitTests` are used to perform method of manufactured solutions studies
of advection outflow boundary conditions with a finite volume discretization. The python mms
scripts can compute the order of convergence of the mesh by running multiple simulations
with increasing discretization. They then check this
convergence against the expected discretization. If the desired order of convergence is not met,
the scripts error out, which is caught by the tester.

!listing test/tests/fvkernels/mms/advective-outflow/tests
