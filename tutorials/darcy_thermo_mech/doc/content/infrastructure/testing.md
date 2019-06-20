# Test System

!---

## Overview

MOOSE includes an extendable test system (python) for executing code with different input files.

Each kernel (or logical set of kernels) should have test(s) for verification.

The test system is flexible, it performs many different operations such as testing for
expected error conditions.

Additionally, custom "Tester" objects can be created.

!---

## Test Setup

Related tests should be grouped into an individual directory and have a consistent naming convention.

It is recommended to organize tests in a hierarchy similar to the application source (i.e., kernels,
BCs, materials, etc).

Tests are found dynamically by matching patterns (highlighted below)

```text
tests/
  kernels/
    my_kernel_test/
      my_kernel_test.e   [input mesh]
      my_kernel_test.i   [input file]
      tests              [test specification file]
      gold/              [gold standard folder for validated solution]
      out.e              [solution]
```

!---

## Test Specification

Test specifications use the hit format, the same format as the standard MOOSE input file.

```text
[Tests]
   [my_kernel_test]
     type    = Exodiff
     input   = my_kernel_test.i
     exodiff = my_kernel_test_out.e
   []

   [kernel_check_exception]
     type       = RunException
     input      = my_kernel_exception.i
     expect_err = 'Bad stuff happened with variable \w+'
   []
 []
```

!---

## Test Objects

+RunApp+: Runs a MOOSE-based application with specified options\\
+Exodiff+: Checks Exodus files for differences within specified tolerances\\
+CSVDiff+: Checks CSV files for differences within specified tolerances\\
+VTKDiff+: Checks VTK files for differences within specified tolerances\\
+RunException+: Tests for error conditions\\
+CheckFiles+: Checks for the existence of specific files after a completed run\\
+ImageDiff+: Compares images (e.g., *.png) for differences within specified tolerances\\
+PythonUnitTest+: Runs python "unittest" based tests\\
+AnalyzeJacobian+: Compares computed Jacobian with finite difference version\\
+PetscJacobianTester+: Compares computed Jacobian using PETSc

!---

## Running Tests

```bash
./run_tests -j 12
```

For more information view the help:

```bash
./run_tests -h
```

!---

## Test Object Options

```bash
./run_tests --dump
```

+input+: The name of the input file\\
+exodiff+: The list of output filenames to compare\\
+abs_zero+: Absolute zero tolerance for exodiff\\
+rel_err+: Relative error tolerance for exodiff\\
+prereq+: Name of the test that needs to complete before running this test\\
+min_parallel+: Minimum number of processors to use for a test (default: 1)\\
+max_parallel+: Maximum number of processors to use for a test

!---

## Notes on Tests

Individual tests should run relatively quickly (2 second rule)

Outputs or other generated files should not be checked into the repository

MOOSE developers rely on application tests when refactoring to verify correctness

- +poor test coverage+ = +higher code failure rate+

!!!
At this point the tests for step3 should be executed, but prior to doing this the following commands
should be run without the audience knowing. The test from step 2 will fail, it will need to be
updated using a GenericConstantMaterial.

```bash
cd ~/projects/moose/tutorials/darcy_thermo_mech/step03_darcy_material
cp ../step02_darcy_pressure/tests/kernels/darcy_pressure/darcy_pressure.i tests/kernels/darcy_pressure/
```
!!!
