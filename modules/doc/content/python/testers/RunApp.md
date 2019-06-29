# RunApp Tester

The RunApp Tester is a concrete intermediate base class whose primary purpose is to assemble a list
of command line arguments and run a MOOSE-based application. The TestHarness handles figuring
out the right name of your application (including the suffix) and passes the correct command line
flags to it.

The RunApp Tester does not add any additional behaviors to the `processResults` method.

## Inspecting Multiple Outputs from a single test

Some tests write out multiple files (e.g. ExodusII and CSV). To maintain simplicity the TestHarness
does not normally inspect multiple files of different types in a single test. However, it is possible
to accomplish this behavior through the use of several parameters controlling whether or not to (re-)execute
a particular test file, and whether or not to remove the expected output before the test runs (normally
handled by the TestHarness to prevent false-positive tests).

!listing moose/test/tests/misc/should_execute/tests
         caption=Demonstration of checking two outputs with one test file
