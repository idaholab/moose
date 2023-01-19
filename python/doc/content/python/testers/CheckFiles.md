# CheckFiles

The `CheckFiles` tester simply checks if some files are output or not during a simulation.
For example, this can be used to make sure that desired output is indeed created by a simulation.

## Options

Test configuration options are added to the `tests` file.

- `check_files`: A list of files that MUST exist

- `check_not_exists`: A list of files that must NOT exist

- `file_expect_out`: A regular expression that must occur in all of the check files
                     in order for the test to be considered passing.

Other test commands & restrictions may be found in the [TestHarness documentation](TestHarness.md).

## Example test configuration in the MOOSE test suite

In this example test configuration, three `CheckFiles` tests and a few other test types are
used to check the proper operation of the mesh splitting functionality. The `CheckFiles` test check
that the split mesh files do indeed exist after performing a mesh split.

!listing test/tests/mesh/splitting/tests
