# PerfGraph

## Overview

Note: this is API/developer documentation intended for those developing code using MOOSE.  For end-user focused documentation see [/PerfGraphOutput.md].

The `PerfGraph` object holds timing data for MOOSE.  The idea behind the design is to create a nested set of timing data that faithfully represents the call structure in MOOSE.

The performance graph is part of an ecosystem of objects:

- `PerfGraph`: Holds the full graph and the routines for printing it out
- `PerfNode`: Makes up each node in the graph and holds timing information for each section of code
- `PerfGuard`: Scope guard used to active and deactivate timers
- `PerfGraphInterface`: An interface class for gaining access to the `PerfGraph` for adding timers and pulling timing data
- [/PerfGraphOutput.md]: Responsible for printing out the graph
- [/PerfGraphData.md]: `Postprocessor` for outputting time from the graph
- [/PerfGraphLivePrint.md]: Object responsible for printing performance information duruing the run

The `PerfGraph` works by utilizing the `TIME_SECTION` macro to specify that the current scope should be timed (see below for more information).  The timed sections are placed in an execution tree and the current "stack" of sections is kept up to date.  When timing starts, a snapshot of both the memory and current time are taken then these are compared when the current scope ends in order to tally time for that section of code.  The `PerfGraphLivePrint` object is watching the stream of what is executing and possibly printing out what is happening if it takes too long (or uses too much memory).  At the end of the run the `PerfGraphOutput` object is responsible for dumping out the relevant information.

!alert warning
`PerfGraph` based timing should NOT be used inside tight compute loops or anything called inside a tight compute loop (i.e. don't use it in `computeQpResidual()`).  It takes about 1e-6 seconds for the timing itself to happen.  That's in the 1 MHz range... meaning that your calculation can't run any faster than that wherever this timer is!  As a general rule... that means that you should have >1000 operations going on inside a timed section.

## Inheriting From `PerfGraphInterface`

To use for timing, make sure that your system inherits from `PerfGraphInterface`.  There are a couple of different constructors for `PerfGraphInterface`:

The first one allows you to pass in a `MooseObject*` and *infer* a "prefix" based on the `type()` of the object (the name of the object).  The "prefix" is prependended to the name of the timed sections to give uniform naming from each object

!listing framework/include/interfaces/PerfGraphInterface.h line=PerfGraphInterface(const MooseObject * moose_object);

The second one allows you to pass in a `MooseObject*` and explicitly set a `prefix`:

!listing framework/include/interfaces/PerfGraphInterface.h line=PerfGraphInterface(const MooseObject * moose_object, const std::string prefix);

The final one is for when your object is NOT a `MooseObject` inherited object.  You explicitly pass in the `PerfLog &` (usually by retrieving it from the `MooseApp`) and explicitly set a `prefix`.

!listing framework/include/interfaces/PerfGraphInterface.h line=PerfGraphInterface(PerfGraph & perf_graph, const std::string prefix = "");

## Logging Levels

The `PerfGraph` relies on loggging "levels" to determine how verbose the output should be.  When timing a section, be sure to set the level appropriately so that users are not inundated with too much noise.  The levels are:

- 0: Just the "root" - the whole application time
- 1: Minimal set of the most important routines (residual/jacobian computation, etc.)
- 2: Important initialization routines (setting up the mesh, initializing the systems, etc.)
- 3: More detailed information from levels `1` and `2`
- 4: This is where the Actions will start to print
- 5: Fairly unimportant, or less used routines
- 6: Routines that rarely take up much time

## Performance Data Types

The data provided in regards to a section or a node in the `PerfGraph` is as follows:

- `SELF`: The time taken (not including children) in seconds
- `CHILDREN`: The time taken by children in seconds
- `TOTAL` The total (self plus children) time taken in seconds
- `SELF_AVG`: The average time taken (not including children) in seconds over all calls
- `CHILDREN_AVG`: The average time taken by children in seconds over all calls
- `TOTAL_AVG`: The total time taken (self plus children) in seconds over all calls
- `SELF_PERCENT`: The percentage of time taken (not including children) relative to the total application time
- `CHILDREN_PERCENT`: The percentage of time taken by children relative to the total application time
- `TOTAL_PERCENT`: The percentage of time taken (self plus children) relative to the total application time
- `SELF_MEMORY`: The memory added (not including children) in Megabytes
- `CHILDREN_MEMORY`: The memory added by children in Megabytes
- `TOTAL_MEMORY`: The total memory added (self plus children) in Megabytes
- `CALLS`: The number of calls

## Timing a Section

There are two different methods for timing: on-the-fly registration and pre-registration.  On-the-fly registration is the preferred method and should be used whenever possible.

### On-The-Fly Section Timing

Timing a section is as simple as using the `TIME_SECTION` macro within a C++ scope.  `TIME_SECTION` can take between one and four arguments.  The single argument version is used when doing pre-registration of sections.  For on-the-fly you invoke `TIME_SECTION` like so:

`TIME_SECTION(section_name, level, live_message="", print_dots=true)`

- `section_name`: The short name of the section.  This is the name used in the final table.  It is normally the function name or some other short name.
- `level`: The logging level
- `live_message`: OPTIONAL - but highly recommended.  This is the message `PerfGraphLivePrint` will print to the screen (if necessary).  It should be descriptive, title-cased, and written in an active way, e.g. "Calculating Lama Heights"
- `print_dots`: OPTIONAL - defaults to true.  This controls whether or not progress dots will be printed for this section.  Only turn this off if printing dots would intermingle with some screen output that is out of MOOSE's control (for instance, in a library that you are calling into).

An example showing the most-often use-case for `TIME_SECTION`:

```c++
void
Dog::clean()
{
  TIME_SECTION("clean", 2, "Cleaning the Dog");
  ...

  {
    TIME_SECTION("soap", 3, "Soaping the Dog");
    ...
  }

  {
    TIME_SECTION("rinse", 3, "Rinsing the Dog");
    ...
  }
  ...
}
```

What `TIME_SECTION` is doing is creating a static variable to hold a `PerfID` that is initialized by registering the section with the `PerfGraphRegistry`.  Since this is a static variable the registration only happens the very first time that line of code is hit .  Every time after that it simply creates `PerfGuard` object using the passed in `PerfID`.  The `PerfGuard` tells the `PerfGraph` about the new scope and the timing is then started for that section.  At the end of the function the `PerfGuard` dies and in the destructor it tells the `PerfGraph` to remove that scope.  Timing this way means that it is exception safe and impossible to "foul up" because there are no "push/pop" methods to match.

#### Early Retrieval

Sections that are registered using the on-the-fly timing method described above (which are the overwhelming majority of sections in the MOOSE framework and modules) are not registered in the `PerfGraph` until the moment they are ran.

This poses a challenge when obtaining data pertaining to said sections: error checking on whether or not a section exists is dependent on whether or not the section has ran yet. The default behavior for obtaining `PerfGraph` section data via `PerfGraph::sectionData()` is to error if a section with that name has not been found.

Let's say you are trying to time Jacobian evaluation time after every timestep, which is stored in the section `FEProblem::computeJacobianInternal`. But, your system only evaluates the Jacobian after the second timestep. If you try to pull section data on `FEProblem::computeJacobianInternal` on `TIMESTEP_END`, the system will error by default after the first evaluation because such a section has not ran (even though it is a valid section). With this, `Perfgraph::sectionData()` takes an optional boolean argument `must_exist` (which defaults to `true`). Setting `must_exist = false` will return zero if the section is not found.

### Pre-registered Timing

This type of timing should _only_ be utilized when absolutely necessary.  The main case where this comes up is timing in base classes that main get instantiated multiple times through different derived classes.  An example is the `Action` base class.

Timing a section using pre-registration is a two part process:

1.  Register the section and save off the `PerfID`
2.  Using the `TIME_SECTION` macro to start timing a `PerfID`

#### Registration

Registering the section of code to be timed is accomplished by calling:

!listing framework/include/interfaces/PerfGraphInterface.h line=registerTimedSection

The `section_name` names the section of code.  The `prefix + section_name` must be globally unique.  `level` is the "log level" of the section.  A higher number represents a more detailed log level.  Here are some quick guidelines for selecting `level`:

`registerTimedSection()` returns a `PerfID` that is a unique identifier that identifies that code section.  This `PerfID` should typically get saved as a member variable of the class that is registering the section... this is normally done by initializing a `PerfID` member variable using `registerTimedSection()` in the initialization list of a constructor like so:

```c++
MyClass::MyClass() : _slow_function_timer(registerTimedSection("slowFunction")) {}
```

#### Timing

Once a timed section is registered and a `PerfID` is captured the section can be timed using the `TIME_SECTION` macro like so:

```c++

void slowFunction()
{
  TIME_SECTION(_slow_function_timer);

  // do all the things
}
```

What `TIME_SECTION` is doing is creating a `PerfGuard` object using the passed in `PerfID`.  The `PerfGuard` tells the `PerfGraph` about the new scope and the timing is then started for that section.  At the end of the function the `PerfGuard` dies and in the destructor it tells the `PerfGraph` to remove that scope.  Timing this way means that it is exception safe and impossible to "foul up" because there are no "push/pop" methods to match.

## Retrieving Section Data

An object that inherits from `PerfGraphInterface` can retrieve the data pertaining to a registered section by calling `perfGraph().sectionData()`. Note the various available data types in [#performance-data-types].

## The `PerfGraph` Internals

The `PerfGraph` object's main purpose is to store the complete call-graph of `PerfNode`s and the current call-stack of `PerfNode`s.  The graph is held by holding onto the `_root_node`.  All other scopes that are pushed into the graph are then children/descendants of the `_root_node`.

The call-stack is held within the `_stack` variable.  The `_stack` is statically allocated to `MAX_STACK_SIZE` and `_current_position` is used to point at the most recent node on the stack.  When a `PerfGuard` tells the `PerfStack` about a new scope, the new scope is added a child to the `PerfNode` that is in the `_current_position`.  `_current_position` is then incremented and the new `PerfNode` is put there. When a scope is removed by the `PerfGuard` the `_current_position` is simply decremented - with no other action being necessary.

In addition, the `_execution_list` is keeping a running list of every section that executes.  This is utilized by `PerfGraphLivePrint` to print messages out that are multiple levels deep.

## Printing

Some other capability the `PerfGraph` has is the ability to print formatted tables displaying the values held in the graph.  These normally shouldn't be called directly, but instead should be accessed using a [/PerfGraphOutput.md] output object.

The `print()` method prints out an indented set of section names and shows their timing like so:

```
Performance Graph:
----------------------------------------------------------------------------------------------------------------------------------------------
|                  Section                 | Calls |   Self(s)  |   Avg(s)   |    %   | Mem(MB) |  Total(s)  |   Avg(s)   |    %   | Mem(MB) |
----------------------------------------------------------------------------------------------------------------------------------------------
| MooseTestApp (main)                      |     1 |      0.008 |      0.008 |   0.66 |       1 |      1.259 |      1.259 | 100.00 |      67 |
|   FEProblem::outputStep                  |     2 |      0.001 |      0.000 |   0.04 |       0 |      0.064 |      0.032 |   5.09 |       8 |
|   Steady::PicardSolve                    |     1 |      0.000 |      0.000 |   0.01 |       0 |      0.717 |      0.717 |  56.92 |      32 |
|     FEProblem::solve                     |     1 |      0.134 |      0.134 |  10.61 |      29 |      0.716 |      0.716 |  56.89 |      32 |
|       FEProblem::computeResidualInternal |    14 |      0.000 |      0.000 |   0.01 |       0 |      0.458 |      0.033 |  36.34 |       1 |
|       FEProblem::computeJacobianInternal |     2 |      0.000 |      0.000 |   0.00 |       0 |      0.125 |      0.062 |   9.91 |       2 |
|     FEProblem::outputStep                |     1 |      0.000 |      0.000 |   0.02 |       0 |      0.000 |      0.000 |   0.02 |       0 |
|   Steady::final                          |     1 |      0.000 |      0.000 |   0.00 |       0 |      0.000 |      0.000 |   0.02 |       0 |
|     FEProblem::outputStep                |     1 |      0.000 |      0.000 |   0.01 |       0 |      0.000 |      0.000 |   0.02 |       0 |
----------------------------------------------------------------------------------------------------------------------------------------------
```

`Calls` is the number of times that section was run. `Self` time is the time actually taken by the section while `Children` time is the cumulative time of all of the sub-sections below that section and `Total` is the sum of the two.  The `Avg` and `%` columns represent the average and percent of the total run-time of the app for the number in the column to the left. `Mem` is the memory (in Megabytes) for the column to the left (Self or Total).

There are also two other ways to print information out about the graph using `printHeaviestBranch()` and `printHeaviestSections()`.  These are described well over on the [/PerfGraphOutput.md] page.
