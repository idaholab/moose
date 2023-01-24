!template load file=far.md.template project=Framework

!template! item key=introduction
MOOSE and MOOSE-based applications are designed to operate as a library of functionality. While
each library may be tailored for solving a certain set of equations, the ability to create arbitrary
simulations exists. This flexibility exists by design within the framework, modules, and applications.
With respect to performing failure analysis, the flexibility is detrimental since there lacks
a well-defined problem to assess. To minimize the possibility of failure for a simulation, various
automated methods exist for developers. This document discusses these features and includes a
list of requirements associated with software failure analysis.
!template-end!

!template! item key=failure-analysis
MOOSE has three primary methods for handling simulation failures that range from input errors to
simulation convergence problems. These potential failures and the associated handling of the failure
are ubiquitous across MOOSE and MOOSE-based applications. The next three sections detail the handling
of these common sources of failures.

1. Input file syntax failure,
2. Input parameter errors, and
3. Convergence failure.

To complement the automatic handling of these three failure mechanisms the MOOSE testing system
includes a mechanism for creating tests to verify that errors are captured and reported correctly.
This testing method is detailed in the [#failure-testing] section.

### Input File Failure

The input file parsing [(see Parser)](Parser.md) system automatically handles syntax mistakes and
reports them as errors. For example, consider the following input file that contains a missing closing
bracket.

!listing parser/hit_error/hit_error.i

If this input file is executed with the application, it will automatically report the error
and associated line number where it occurred as follows.

```text
hit_error.i:5: missing closing '[]' for section
```

### Input Parameter Errors id=input-parameter-errors

The input parameter system (see [utils/InputParameters.md]) is the second step in input file
parsing. The system details the available inputs for an object. The system allows for
parameters to be marked as required, provide a default, or check for correct range to name a few.
For example, consider the `validParams` function below that defines a required parameter "D" that
must be supplied in an input file.

!listing test/src/kernels/CoeffParamDiffusion.C start=MooseDocs::start end=MooseDocs::end include-start=False

If an input file does not include this parameter, as shown below then it will provide an error
with details regarding the missing parameter.

!listing param_error/param_error.i block=Kernels

```text
param_error.i:10: missing required parameter 'Kernels/diffusion/D'
	Doc String: "The diffusivity coefficient."
```

### Convergence Failure

MOOSE includes automatic methods to handle convergence failures during the numeric solve. If those
attempts fail, it will exit with an error indicating of the failed solve and the reason. By default
if a transient simulation fails to solve a time step, the timestep will automatically be cut and the
solve re-attempted. This cutback will continue until the solve converges or if the minimum allowed
timestep is reached.

For example, the following input when executed will demonstrate the behavior. This input file
includes a custom `TimeStepper` block, but by default a similar behavior exists.

!listing cutback_factor_at_failure/constant_dt_cutback.i block=Executioner

When executed this input file at time step 3 fails to converge, the timestep ("dt") is
cut by the supplied factor and the solve is re-attempted. In both the converged and non-converged
iterations the reason for the resulting solve is displayed.


```text
Time Step 3, time = 0.3, dt = 0.1
 0 Nonlinear |R| = 7.103698e-02
      0 Linear |R| = 7.103698e-02
      1 Linear |R| = 1.154171e-03
      2 Linear |R| = 4.325671e-06
      3 Linear |R| = 2.434939e-08
  Linear solve converged due to CONVERGED_RTOL iterations 3
 1 Nonlinear |R| = 2.429061e-08
      0 Linear |R| = 2.429061e-08
      1 Linear |R| = 2.035627e-10
      2 Linear |R| = 9.270880e-13
      3 Linear |R| = 6.368586e-15
  Linear solve converged due to CONVERGED_RTOL iterations 3
 2 Nonlinear |R| = 6.368769e-15
Nonlinear solve converged due to CONVERGED_FNORM_RELATIVE iterations 2
 Solve Did NOT Converge!
Aborting as solve did not converge

Time Step 3, time = 0.28, dt = 0.08
 0 Nonlinear |R| = 7.103698e-02
      0 Linear |R| = 7.103698e-02
      1 Linear |R| = 8.875771e-04
      2 Linear |R| = 3.163939e-06
      3 Linear |R| = 1.554863e-08
  Linear solve converged due to CONVERGED_RTOL iterations 3
 1 Nonlinear |R| = 1.565086e-08
      0 Linear |R| = 1.565086e-08
      1 Linear |R| = 1.120313e-10
      2 Linear |R| = 4.275206e-13
      3 Linear |R| = 2.854434e-15
  Linear solve converged due to CONVERGED_RTOL iterations 3
 2 Nonlinear |R| = 2.874867e-15
Nonlinear solve converged due to CONVERGED_FNORM_RELATIVE iterations 2
 Solve Converged!
```

### Failure Testing id=failure-testing

In general, failures are tested using a test type of `RunException` (see [framework_stp.md]). An
example of such as test is provided below, which is a test that exists for the previous
input parser example in [#input-parameter-errors]. By default all `RunException` tests
are listed below in the list in of requirements ([#failure-analysis-requirements]) that comprise
failure analysis.

!listing param_error/tests

!template-end!

!template item key=failure-analysis-requirements
!sqa requirements link=True collections=FAILURE_ANALYSIS category=framework
