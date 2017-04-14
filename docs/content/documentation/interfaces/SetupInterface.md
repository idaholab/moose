# SetupInterface (execute_on)

Most user-facing objects in MOOSE inherit from the SetupInterface class. This class provides two
features to objects. Foremost, it provides the the "execute_on" parameter, which, as the name suggests, dictates when the object is to be executed. Secondly, it adds virtual setup methods that allow derived classes to perform setup applications prior to execution.

## Execute On

Any object inheriting from the SetupInterface will have an "execute_on" parameter that can be set
to various flags, the most common flags are listed below.

| Execute Flag | Description |
| - | - |
INITIAL | Prior to the first time step.
TIMESTEP_BEGIN | Prior to the solve for each time step.
NONLINEAR | Prior do each non-linear iteration during the solve.
LINEAR | Prior do each linear iteration during the solve.
TIMESTEP_END | After the solve for each time step.
SUBDOMAIN | Executes when the subdomain (i.e., "blocks") change during calculations.

The "execute_on" parameter can be set to a single flag or multiple flags. For example, it may be desirable to only execute an object initially because the state of the auxiliary computation does not vary. In the input file snippet below, the [ElementLengthAux](framework/ElementLengthAux.md)
computation only needs to be computed initially, thus the "exeucte_on" parameter is set as such.

!input test/tests/auxkernels/element_length/element_length.i block=AuxKernels

Alternatively, it is often desirable to run a computation with multiple execute flags. For example,
in the input file snippet below a [TimePeriod](framework/TimePeriod.md) control object that is
responsible for enabling in [Damper](/Dampers/index.md) object needs to be run initially
and prior to each timestep to guarantee that the damper is enabled when desired.

!input test/tests/controls/time_periods/dampers/control.i block=Controls

Depending on the system these options or others will be available, since as discussed in [Creating Custom Execute Flags](#creating-custom-execute-flags) custom flags may be added. The complete list
of execution flags is provided by MOOSE are listed in the "resisterExecFlags" function.

!text framework/src/base/Moose.C re_start=void\nregisterExecFlags re_end=void\nsetSolverDefaults

## Modifying Execute On
When creating objects that inherit from SetupInterface it is possible to set, add, or remove available execute flags using the following MooseUtil functions.

* `MooseUtils::setExecuteOnFlags` <br> This function **sets** the defaults based on the existing executes flags.

* `MooseUtils::addExecuteOnFlags` <br> This utility function **adds** flags to the list of available execute flags.

* `MooseUtils::addExecuteOnFlags` <br> This function **removes** flags to the list of available execute flags.

For example, as shown below, the Output system works with two additional execute flags ("failed" and "final") and
modifies the default flags to be "initial" and "timestep_end".

!text framework/src/outputs/Output.C re_start=\s*MooseUtils::addExecuteOnFlags re_end=MooseUtils::setExecuteOnFlags.*?;\n include_end=True

## Virtual Setup Methods

The SetupInterface includes virtual methods that correspond to the primary execute flags
with MOOSE, these methods are listed in the header as shown here.

!text framework/include/base/SetupInterface.h re_start=\n.*?\n.*?\n.*?\n\s*virtual\s+void\s+initialSetup re_end=subdomainSetup\(\);$ include_end=True

In general, these methods should be utilized to perform "setup" procedures prior to the calls to
execute for the corresponding execute flag.

!!!note
    A few of the methods were created prior to the execute
    flags, thus the names do not correspond but they remain as is to keep the API consistent: the
    "jacobianSetup" methods is called prior to the "NONLINEAR" execute flag and the "residualSetup" is
    called prior to the "LINEAR" execute flag.


## Creating Custom Execute Flags
It is possible to create custom execute flags for an application. To create at utilize a custom
execute flag the following steps should be followed.

### 1. Declare and Define an Execute Flag

Within your application a new global `const` should be declared in a header file. For example, within the test application for MOOSE, there is a header (MooseTestAppTypes.h) that declares a
new flag (EXEC_JUST_GO).

!text test/include/base/MooseTestAppTypes.h

This new global must be defined, which occurs in the corresponding source file.

!text test/src/base/MooseTestAppTypes.C

### 2. Register the Execute Flag
After the new flag(s) are defined and defined, it must be registered with MOOSE. The registration
works similar to registering objects. In your application there is a static function named
"registerExecFlags" that has a definition in the application header and a definition in the corresponding source file. Within the definition (C-file) the flag should be registered by calling
registerExecFlag. For example, the MooseTestApp contains the following.

!text test/include/base/MooseTestApp.h line=registerExecFlags strip-leading-whitespace=True

!text test/src/base/MooseTestApp.C re_start=void\nMooseTestApp::registerExecFlags()

!!!note "NOTE: registerExecFlag function"
    The first argument to the `registerExecFlag` function should be the flag that was created in the previous step. The second argument is the string that you would like to add to the input file syntax.

### 3. Add the Execute Flag to InputParameters
After a flag is registered, it must be made available to the object(s) in which are desired to be executed with the custom flag. This is done by adding this new flag to an existing objects valid parameters. For example, the following adds the "EXEC_JUST_GO" flag to a postprocessor.

!text test/src/postprocessors/TestPostprocessor.C re_start=template\s<>\nInputParameters re_end=TestPostprocessor::TestPostprocessor


### 4. Use the Execute Flag
Depending on what type of custom computation is desired, various MOOSE execution calls accept execution flags, which will spawn calculations. For example, the TestSteady contains the following, which uses the custom flag that was defined and registered.

!text test/src/executioners/TestSteady.C re_start=void\nTestSteady::postSolve
