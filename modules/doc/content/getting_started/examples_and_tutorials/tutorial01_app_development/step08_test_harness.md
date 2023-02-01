!content pagination previous=tutorial01_app_development/step07_parallel.md
                    next=tutorial01_app_development/step09_mat_props.md
                    margin-bottom=0px

# Step 8: Write a Regression Test

When developing software for scientific computation, it is prudent to maintain an archive of certified solutions. This way, developers can easily check that their application continues to produce good results as the source code expands and refactors. This step introduces the [application_development/test_system.md], which provides tools for developing [*Regression Tests*](https://en.wikipedia.org/wiki/Regression_testing) based on input files and their outputs. For the demonstration, two different tests of the `DarcyPressure` class will be created using this system.

## TestHarness

The application testing system is provided by the [`TestHarness`](python/TestHarness/TestHarness.py) Python module. This system lends its users a rich set of testing methods, such as testing input conditions that are supposed to invoke an error and checking whether that error does indeed occur. Every `MooseObject` should have automated test(s) for rapid verification. The primary approach used in MOOSE is *gold file* references, which involves executing an input file and comparing the outputs---usually in the form of `.csv` and/or ExodusII files---to a database of expected outputs stored in a directory named "gold." An example of this test approach was demonstrated in Step 1 when the [simple diffusion test](getting_started/examples_and_tutorials/tutorial01_app_development/step01_moose_app.md#test) was ran.

### Test Setup id=setup

`TestHarness` requires that all test related files be organized in a standard format. By now, the reader must have noticed the `test` directory in their application root. All tests should be grouped logically into individual directories in `test/tests` and have a consistent naming convention. The folders in `test/tests` are usually hierarchically structured, similarly as to how those in the `src` and `include` directories are. For example, test files for the `Diffusion` class are located in a subdirectory of `test/test/kernels`, since it is a `Kernel` object. The object's designated test directory will then contain the following:

1. Input files designed to test the application.
1. A `gold` directory containing the certified results to be matched by outputs.
1. A test specification file that provides the instructions to `TestHarness`.

### Test Specification

Test specification files are [!ac](HIT) formatted ones that define the tests to be run. By default, `TestHarness` searches the directory tree for a file named "tests" to read the specs from. The familiar [!ac](HIT) syntax used in these files is demonstrated by the specification for the simple diffusion test:

!listing tutorials/tutorial01_app_development/step08_test_harness/test/tests/kernels/simple_diffusion/tests
         link=False
         remove=Tests/issues Tests/design Tests/test/requirement

The `type` parameter specifies one of the available test methods. In the above case, the `Exodiff` object is used, which requires an input file be specified with the `input` parameter and an ExodusII formatted gold file be specified with the `exodiff` parameter. The [#demo] section will show how to use the `Exodiff` and `RunException` testers on the `DarcyPressure` class. For a complete list of test objects, please see the discussion about [testers](TestHarness.md#testers).

!alert tip title=Tests should be simple and cheap.
Although it is important that regression tests be comprehensive, ideally, they should also be as simple as possible and not have to process any unnecessary source code. The input file should create as few objects as possible beyond one for the class being tested. MOOSE developers abide by the "2 second rule," i.e., a test should complete in no more than two seconds.

### Run Tests

The [`run_tests`](tutorials/tutorial01_app_development/step08_test_harness/run_tests language=python) Python script located in the application root can be executed to invoke `TestHarness` on all test specifications it finds in `test/tests`. The basic command was already demonstrated in [Step 1](getting_started/examples_and_tutorials/tutorial01_app_development/step01_moose_app.md#test):

!include commands/run_tests.md

This will execute the [`simple_diffusion.i`](tutorials/tutorial01_app_development/step08_test_harness/test/tests/kernels/simple_diffusion/simple_diffusion.i) file and conduct the `Exodiff` procedure. When the [#test-demo] section introduces two new specifications, it will be shown that the above command will cause those tests to run as well.

!alert! note title=Tests can be executed in parallel (in a variety of ways).
The `run_tests` script has several command-line options that can be useful to developers (see [TestHarness.md#moreoptions]). The most common is the "`-j <n>`" argument for specifying the number of tests to run simultaneously. When used, each of the `n` processors will independently run one of the test specifications, which decreases the total time it takes to run all tests.

A given test specification can also be made to run in parallel and/or with multithreading in the manner discussed in the [previous step](tutorial01_app_development/step07_parallel.md). So in addition to running multiple unique tests simultaneously by specifying the `-j` argument, each can invoke [!ac](MPI) using `-p <n>` and/or threads using `--n-threads <n>` (or `--n-threads=<n>`). For example, the command below will run four test jobs on two processors and two threads each.

!include commands/run_tests.md
         replace=['-j4', '-j4 -p2 --n-threads\=2']

Regression tests are usually cheap, and so individual parallelism generally won't increase overall testing speed. But besides possibly increasing performance, using the `-p` command is also a test in itself, since it ought to be confirmed that the application is capable of producing the expected results in parallel. See the discussion about [parallel test execution](TestHarness.md#parallel) for more information.
!alert-end!

## Demonstration id=demo

This demonstration will show how to create the [three test setup items](#setup) that were mentioned earlier in order to have an official test of the `DarcyPressure` kernel. But, first, notice that the weak form requires that $\mu \; {=}\mathllap{\small{/}\,} \; 0$:

!equation
(\nabla \psi, \dfrac{K}{\mu} \nabla p) = 0

If $\mu = 0$, the residual would yield a [`NaN`](https://en.wikipedia.org/wiki/NaN) value and the application would crash. Thus, the source code shall be modified to prevent this condition and a second test will be created with the input `viscosity = 0` to verify that a proper, unambiguous error occurs.

### Source Code id=source-demo

To be sure, one can confirm that the "`viscosity = 0`" input leads to a crash by adding it to the `[diffusion]` block in `pressure_diffusion.i` and executing the input file. Upon doing so, the following message is reported to the terminal:

!listing language=bash
Nonlinear solve did not converge due to DIVERGED_FNORM_NAN iterations 0
 Solve Did NOT Converge!
Aborting as solve did not converge

The status `DIVERGED_FNORM_NAN` indicates that the residual was indeed `NaN` due to division by zero. Please be sure to remove this erroneous input, or simply enter `git restore pressure_diffusion.i`, before proceeding.

Since it is clear that the zero-valued viscosity input is invalid, an error should occur from within the `DarcyPressure` class by invoking the [`paramError()` method](framework_development/sanity_checking.md#missing-and-incorrect-parameters optional=True) on the `"viscosity"` parameter. Add this call to the constructor method in `DarcyPressure.C`:

!listing tutorials/tutorial01_app_development/step08_test_harness/src/kernels/DarcyPressure.C
         link=False
         start=DarcyPressure::DarcyPressure
         end=ADRealVectorValue

From now on, if a user tries to input `viscosity = 0`, the message printed to the terminal will be more explicit about what went wrong: *The viscosity must be a non-zero real number.* Do not modify any other parts of `DarcyPressure.C`. Lastly, be sure to recompile the application:

!include commands/make.md

### Input File id=input-demo

The simple diffusion test is a great example of a regression test that is simple, cheap, and provides sufficient verification of the class being tested, i.e., `Diffusion`. Specifically, it creates very few objects besides a `Diffusion` one, the mesh is coarse enough to produce a number of [!ac](DOFs) that can be solved in almost no time, but fine enough to result in a fairly complex system of equations, and the correct solution to the [!ac](BVP) (zero on the left and unity on the right) is obvious, because the geometry (a 1-by-1 square) is a direct mapping of it. Therefore, to make matters easy, the `simple_diffusion.i` file will be copied and then modified to use a `DarcyPressure` object.

Start by creating a directory to store test files related to the `DarcyPressure` class:

!include commands/mkdir.md
         replace=['<d>', 'test/tests/kernels/darcy_pressure']

In this folder, create a file named `darcy_pressure_test.i` and add the inputs given in [darcy-kernel-test]. Here, the value input for the permeability is that for the 1 mm steel sphere medium and the viscosity is left as the default value $\mu_{f}$. The `"exodus"` parameter was set to `true` in the `[Outputs]` block so that an `Exodiff` object can reference the output file.

!listing tutorials/tutorial01_app_development/step08_test_harness/test/tests/kernels/darcy_pressure/darcy_pressure_test.i
         link=False
         id=darcy-kernel-test
         caption=Input file to test the `DarcyPressure` class with an `Exodiff` object.

Execute the input file in [darcy-kernel-test], confirm that the solver completes, and that a file named `darcy_pressure_test_out.e` is generated:

!listing language=bash
cd ~/projects/babbler/test/tests/kernels/darcy_pressure
../../../../babbler-opt -i darcy_pressure_test.i

Next, create a file named `zero_viscosity_error.i` in the same folder and add the inputs given in [darcy-error-test]. Here, `viscosity = 0` was input to deliberately incur the expected error. Also, since no actual [!ac](FE) problem is going to be solved, the `"solve"` parameter was set to `false` in the `[Problem]` block, which will increase performance by not processing tasks that are unnecessary for this particular test.

!listing tutorials/tutorial01_app_development/step08_test_harness/test/tests/kernels/darcy_pressure/zero_viscosity_error.i
         link=False
         id=darcy-error-test
         caption=Input file to test the `DarcyPressure` class with a `RunException` object.

Execute the input file in [darcy-error-test] and confirm that the application reports the error message that was passed to the `paramError()` method:

!listing language=bash
cd ~/projects/babbler/test/tests/kernels/darcy_pressure
../../../../babbler-opt -i zero_viscosity_error.i

### Results id=result-demo

Visualize the results of the `darcy_pressure_test.i` input file with PEACOCK:

!include commands/peacock_r.md
         replace=['<d>', 'test/tests/kernels/darcy_pressure',
                  '<e>', 'darcy_pressure_test_out']

Confirm that the solution resembles that which is shown in [results]. Notice that the solution satisfies the BVP: $p(0, y) = 0$ and $p(1, y) = 1$, $\forall \, y \in [0, 1]$. Also, notice that the solution directly maps to the spatial coordinates along the $x$-direction as expected. This result is clearly indicated by the contour colors displayed by [results]. Thus, one may declare that the solution is good and, therefore, that the `DarcyPressure` class is functioning properly.

!media tutorial01_app_development/step08_result.png
       style=width:64%;margin-left:auto;margin-right:auto;
       id=results
       caption=Rendering of the solution produced by the `DarcyPressure` test.

### Test id=test-demo

Since the results of the `darcy_pressure_test.i` input file have been deemed good, the ExodusII output can now become a certified gold file:

!include commands/new_gold.md
         replace=['<d>', 'kernels/darcy_pressure',
                  '<e>', 'darcy_pressure_test_out']

Next, create a file named `tests` in `test/tests/kernels/darcy_pressure` and add the inputs given in [darcy-test-spec]. The first test is defined in the `[test]` block, which creates an `Exodiff` object that will attempt to match the output from `darcy_pressure_test.i` to the gold file just created. The second test is defined in the `[zero_viscosity_error]` block, which creates a `RunException` object that attempts to match the error message incurred by `zero_viscosity_error.i` to the string specified for the `expect_err` parameter.

!listing tutorials/tutorial01_app_development/step08_test_harness/test/tests/kernels/darcy_pressure/tests
         link=False
         remove=Tests/issues Tests/design Tests/test/requirement Tests/zero_viscosity_error/requirement
         id=darcy-test-spec
         caption=Test specification file for the `DarcyPressure` class.

Finally, all tests, including the new ones, can be run by `TestHarness`:

!include commands/run_tests.md

If the tests passed, the terminal output should look something like that shown below.

```
test:kernels/darcy_pressure.test .......................................................................... OK
test:kernels/simple_diffusion.test ........................................................................ OK
test:kernels/darcy_pressure.zero_viscosity_error .......................................................... OK
--------------------------------------------------------------------------------------------------------------
Ran 3 tests in 0.4 seconds. Average test time 0.1 seconds, maximum test time 0.1 seconds.
3 passed, 0 skipped, 0 pending, 0 failed
```

### Commit id=commit-demo

Update the `DarcyPressure.C` file and add all of the new test files to the git tracker:

!include commands/git_add.md
         replace=['*', 'src/kernels/DarcyPressure.C test/tests/kernels/darcy_pressure']

Now, commit and push the changes to the remote repository:

!include commands/git_commit.md
         replace=['<m>', '"implemented a zero-viscosity error and created tests for the Darcy pressure kernel"']

!content pagination previous=tutorial01_app_development/step07_parallel.md
                    next=tutorial01_app_development/step09_mat_props.md
