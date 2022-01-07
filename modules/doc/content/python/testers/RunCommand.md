# RunCommand

The `RunCommand` tester allows for specifying directly the command line to run for a test.
This can be done to run a python script, to run additional commands in addition to the
moose executable, or any other command that fits in a command line input.
The syntax of the command follows the `bash` syntax, with regards to running multiple
commands on the same line for example.

## Options

Test configuration options are specified in the `tests` file.

- `command`: The command line to execute for this test

- `test_name`: The name of the test - populated automatically


Other test commands & restrictions may be found in the [TestHarness documentation](TestHarness.md).

## Example test configuration in MOOSE test suite

In this example, `RunCommand` tests are ran after `RunApp` tests to run a custom Python script
to postprocess the output of the [PerfGraphReporter.md].

!listing test/tests/reporters/perf_graph_reporter/tests
