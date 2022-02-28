# RunException

The `RunException` tester is used to check exceptions and expected error messages. The
application is intentionally placed in a wrong/erroring configuration for the test, and
the test makes sure that the problem is indeed caught and the simulation does not proceed.

## Options

Test configuration options are specified in the `tests` file.

- `expect_err`: A regular expression or literal string that must occur in the output (see `match_literal`). (Test may terminate unexpectedly and be considered passing)

- `expect_assert`: DEBUG and DEVEL MODE ONLY: A regular expression that must occur in the output. (Test may terminate unexpectedly and be considered passing)

- `should_crash`: Indicates that the test is expected to crash or otherwise terminate early. Defaults to True


Tests with an `expect_assert` parameter will only be run in DEBUG and DEVEL mode, not in OPT mode.
Other test commands & restrictions may be found in the [TestHarness documentation](TestHarness.md).

## Example test configuration in the MOOSE test suite

In this example, a single `Exodiff` test is used to examine the outputs of the `MaterialADConverter`.
But six `RunException` tests are then used to examine all the potential user input errors that could
be made and that the `MaterialADConverter` should error out on.

!listing test/tests/materials/functor_properties/ad_conversion/tests
