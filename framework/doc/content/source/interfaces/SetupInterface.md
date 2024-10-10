# SetupInterface

Most user-facing objects in MOOSE inherit from the `SetupInterface` class. This class provides two
features to objects:

- The `execute_on` parameter, which, as the name suggests, dictates when the object is to be executed.
- Virtual setup methods that allow derived classes to perform setup applications prior to execution.

## Execute On

Any object inheriting from the SetupInterface, that adds the `SetupInterface::validParams()` within its
own parameters, will have an `execute_on` parameter that can be set to various flags.
The default (and thus most common) flags are listed below:

| Execute Flag | Description |
| :- | :- |
`NONE` | Never executed.
`INITIAL` | Prior to the first time step.
`LINEAR` | Prior to each residual evaluation.
`NONLINEAR` | Prior to each Jacobian evaluation.
`NONLINEAR_CONVERGENCE` | Prior to each nonlinear convergence check. Note that `NONLINEAR` is not appropriate for this purpose since that only executes if it has already been determined that another nonlinear step is to be taken.
`TIMESTEP_END` | After the solve for each time step.
`TIMESTEP_BEGIN` | Prior to the solve for each time step.
`FINAL` | At the end of the entire simulation.
`CUSTOM` | At user specified instants.
`ALWAYS` | Union of all the above flags.

Note that other flags exist, since it is possible for applications to add additional
flags to those added by the core MOOSE framework (see [#creating-custom-execute-flags] below).
Documentation on these custom flags should appear within the associated application documentation;
please contact the application's developers if not found.

The `execute_on` parameter can be set to a single flag or multiple flags. For example, it may be
desirable to only execute an object initially because the state of the auxiliary computation does not
vary. In the input file snippet below, the [ElementLengthAux](/ElementLengthAux.md) computation only
needs to be computed initially, and thus the `execute_on` parameter is set as such.

!listing test/tests/auxkernels/element_length/element_length.i block=AuxKernels

Alternatively, it is often desirable to run a computation with multiple execute flags. For example,
in the input file snippet below a [TimePeriod](/TimePeriod.md) control object that is responsible for
enabling a [Damper](/Dampers/index.md) object needs to be run initially and prior to each timestep
to guarantee that the damper is enabled when desired.

!listing test/tests/controls/time_periods/dampers/control.i block=Controls

Depending on the system these options or others will be available, since, as discussed in
[Creating Custom Execute Flags](#creating-custom-execute-flags), custom flags may be added. The
list of execution flags registered by MOOSE are listed in the `registerExecFlags` function:

!listing framework/src/base/MooseApp.C start=MooseApp::registerExecFlags() end=void

Each type of MOOSE object can choose its own default value for `execute_on`, which
can be found in their `validParams()` method. Some examples are as follows:

| Object Base Class | Default `execute_on` Values |
| :- | :- |
| `AuxKernel` | `LINEAR`, `TIMESTEP_END` |
| `Postprocessor` | `TIMESTEP_END` |
| `UserObject` | `TIMESTEP_END` |
| `Output` | `INITIAL`, `TIMESTEP_END` |
| `Control` | `INITIAL`, `TIMESTEP_BEGIN` |
| `MultiApp` | `TIMESTEP_BEGIN` |
| `Transfer` | (same as corresponding `MultiApp`) |

## Modifying Execute On

When creating objects that inherit from `SetupInterface` it is possible to set, add, or remove
available execute flags by retrieving and then modifying the `ExecFlagEnum` parameter. For example,
consider the snippet below (see [Output.C](/framework/src/outputs/Output.C)).

!listing framework/src/outputs/Output.C start=ExecFlagEnum end=setDocString include-end=True

First, the `execute_on` parameter is retrieved for modification by using the "set" method. Notice, that a
second boolean argument is passed to "set": this second flag enables "quiet mode". Quiet mode allows
this default modification to not count as a user modification for the purposes of `InputParameters::isParamSetByUser`.

Second, the two new execution flags are added (`EXEC_FINAL` and `EXEC_FAILED`); therefore these
additional options are available to all classes (all `Output` objects in this case) that
inherit from this object.

Third, the default active flags are set to `EXEC_INITIAL` and `EXEC_TIMESTEP_END`, which
are the defaults for all `Output` objects.

Finally, the documentation string for the `execute_on` parameter for the `Output` objects is
updated to reflect the changes made to the parameter. The `ExecFlagEnum` has a convenience function
that generates a documentation string that includes the URL to this page.


## Virtual Setup Methods

The SetupInterface includes virtual methods that correspond to the primary execute flags
with MOOSE, these methods are listed in the header as shown here.

!listing framework/include/interfaces/SetupInterface.h
         start=static InputParameters validParams()
         end=customSetup
         include-end=True
         include-start=False
         strip-leading-whitespace=True

In general, these methods should be utilized to perform "setup" procedures prior to the calls to
execute for the corresponding execute flag.

!alert note
A few of the methods were created prior to the execute flags, thus the names do not correspond but
they remain as is to keep the API consistent: the "jacobianSetup" methods is called prior to the
"NONLINEAR" execute flag and the "residualSetup" is called prior to the "LINEAR" execute flag.

There is also a generic setup function "customSetup" that takes an execute flag as the argument.
This function is called by MOOSE when performing evaluations of objects on the custom execute flags
in [Creating Custom Execute Flags](#creating-custom-execute-flags).

!alert warning title=Note on the "customSetup" function
This function is not called on *initial*, *timestep_begin*, *subdomain*, *nonlinear* and *linear*.
Setup operations for those execute flags should be implemented in *initialSetup*, *timestepSetup*,
*subdomainSetup*, *jacobianSetup* and *residualSetup* functions respectively.

## Creating Custom Execute Flags

It is possible to create custom execute flags for an application. To create and utilize a custom
execute flag, the following steps should be followed.

### 1. Register an Execute Flag

Within your application a new global `const` should be declared in a header file. For example, within
the `LevelSetApp` within MOOSE modules, the header `LevelSetTypes.h` declares the new
flags `EXEC_ADAPT_MESH` and `EXEC_COMPUTE_MARKERS`:

!listing modules/level_set/include/base/LevelSetTypes.h

This new global must be registered, which occurs in the corresponding source file using the `registerExecFlag()` macro defined in `ExecFlagRegistry.h`:

!listing modules/level_set/src/base/LevelSetTypes.C

### 2. Add the Execute Flag to InputParameters

After a flag is registered, it must be made available to the object(s) that are desired to be
executed with the custom flag. This is done by adding this new flag to an existing object's valid
parameters. For example, the following adds the `EXEC_ADAPT_MESH` and `EXEC_COMPUTE_MARKERS` flags to a `Transfer` object
and then sets the default to be `{LevelSet::EXEC_COMPUTE_MARKERS, LevelSet::EXEC_ADAPT_MESH}`:

!listing modules/level_set/src/transfers/LevelSetMeshRefinementTransfer.C strip-leading-whitespace=1 start=ExecFlagEnum end=params.set<bool>


### 3. Use the Execute Flag

Depending on what type of custom computation is desired, various MOOSE execution calls accept
execution flags, which will spawn calculations. For example, the `LevelSetProblem` contains
a custom method that uses the `EXEC_ADAPT_MESH` and `EXEC_COMPUTE_MARKERS` flags to perform
an additional [`MultiAppTransfer`](Transfers/index.md) execution:

!listing modules/level_set/src/base/LevelSetProblem.C strip-leading-whitespace=1 line=LevelSet::EXEC_ADAPT_MESH
