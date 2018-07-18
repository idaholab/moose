# PerfGraph

## Overview

The `PerfGraph` object holds timing data for MOOSE.  The idea behind the design is to create a nested set of timing data that faithfully represents the call structure in MOOSE.

The performance graph is part of an ecosystem of objects:

- `PerfGraph`: Holds the full graph and the routines for printing it out
- `PerfNode`: Makes up each node in the graph and holds timing information for each section of code
- `PerfGuard`: Scope guard used to active and deactivate timers
- `PerfGraphInterface`: An interface class for gaining access to the `PerfGraph` for adding timers and pulling timing data
- [/PerfGraphOutput.md]: Responsible for printing out the graph
- [/PerfGraphData.md]: `Postprocessor` for outputing time from the graph

The `PerfGraph` works by registering "sections" of code using unique (`std::string`) names.  The registration of a section returns a `PerfID` unique ID that is then used when referring to that section of code for starting and stopping timing.  It's normal to save the `PerfID` in a member variable as a variable called `*_timer`.

!alert warning
`PerfGraph` based timing should NOT be used inside tight compute loops or anything called inside a tight compute loop (i.e. don't use it in `computeQpResidual()`).  It takes about 1e-6 seconds for the timing itself to happen.  That's in the MHz range... meaning that your calculation can't run any faster than that wherever this timer is!  As a general rule... that means that you should have >1000 operations going on inside a timed section.

## Inheriting From `PerfGraphInterface`

To use for timing, make sure that your system inherits from `PerfGraphInterface`.  There are a couple of different constructors for `PerfGraphInterface`:

The first one allows you to pass in a `MooseObject*` and *infer* a "prefix" based on the `type()` of the object (the name of the object).  The "prefix" is prependended to any call to `registerTimedSection()` to give uniform naming from each object

!listing framework/include/interfaces/PerfGraphInterface.h line=PerfGraphInterface(const MooseObject * moose_object);

The second one allows you to pass in a `MooseObject*` and explicitly set a `prefix`:

!listing framework/include/interfaces/PerfGraphInterface.h line=PerfGraphInterface(const MooseObject * moose_object, const std::string prefix);

The final one is for when your object is NOT a `MooseObject` inherited object.  You explicitly pass in the `PerfLog &` (usually by retrieving it from the `MooseApp`) and explicitly set a `prefix`.

!listing framework/include/interfaces/PerfGraphInterface.h line=PerfGraphInterface(PerfGraph & perf_graph, const std::string prefix = "");

## Timing a Section

Timing a section is a two part process:

1.  Register the section and save off the `PerfID`
2.  Using the `TIME_SECTION` macro to start timing a `PerfID`

### Registration

Registering the section of code to be timed is accomplished by calling:

!listing framework/include/interfaces/PerfGraphInterface.h line=registerTimedSection

The `section_name` names the section of code.  The `prefix + section_name` must be globally unique.  `level` is the "log level" of the section.  A higher number represents a more detailed log level.  Here are some quick guidelines for selecting `level`:

- 0: Just the "root" - the whole application time
- 1: Minimal set of the most important routines (residual/jacobian computation, etc.)
- 2: Important initialization routines (setting up the mesh, initializing the systems, etc.)
- 3: More detailed information from levels `1` and `2`
- 4: This is where the Actions will start to print
- 5: Fairly unimportant, or less used routines
- 6: Routines that rarely take up much time


`registerTimedSection()` returns a `PerfID` that is a unique identifier that identifies that code section.  This `PerfID` should typically get saved as a member variable of the class that is registering the section... this is normally done by initializing a `PerfID` member variable using `registerTimedSection()` in the initialization list of a constructor like so:

```c++
MyClass::MyClass() : _slow_function_timer(registerTimedSection("slowFunction")) {}
```

### Timing

Once a timed section is registered and a `PerfID` is captured the section can be timed using the `TIME_SECTION` macro like so:

```c++

void slowFunction()
{
  TIME_SECTION(_slow_function_timer);

  // do all the things
}
```

What `TIME_SECTION` is doing is creating a `PerfGuard` object using the passed in `PerfID`.  The `PerfGuard` tells the `PerfGraph` about the new scope and the timing is then started for that section.  At the end of the function the `PerfGuard` dies and in the destructor it tells the `PerfGraph` to remove that scope.  Timing this way means that it is exception safe and impossible to "foul up" because there are no "push/pop" methods to match.

## Retrieving Time

An object that inherits from `PerfGraphInterface` can retrieve the time for a registered section by calling `_perf_graph.getTime()` (or `_perf_graph.getSelf`/`Children`/`TotalTime()`).  These functions return a reference to where the time will be updated for that particular section.  In the normal MOOSE way, the object should hold onto that reference and just use the value of it when it needs to know the time a section has taken.  There is one small issue though... `_perf_graph.updateTiming()` should be called to ensure that the time held by the referene is up to date.

## The `PerfGraph` Internals

The `PerfGraph` object's main purpose is to store the complete call-graph of `PerfNode`s and the current call-stack of `PerfNode`s.  The graph is held by holding onto the `_root_node`.  The `_root_node` (which is named `App`) is created at the time the `PerfGraph` is created (in the `MooseApp` constructor).  All other scopes that are pushed into the graph are then children/descendents of the `_root_node`.

The call-stack is held within the `_stack` variable.  The `_stack` is statically allocated to `MAX_STACK_SIZE` and `_current_position` is used to point at the most recent node on the stack.  When a `PerfGuard` tells the `PerfStack` about a new scope the new scope is added a child to the `PerfNode` that is in the `_current_position`.  `_current_position` is then incremented and the new `PerfNode` is put there.

When a scope is removed by the `PerfGuard` the `_current_position` is simply decremented - with no other action being necessrry.

## Printing

Some other capability the `PerfGraph` has is the ability to print formatted tables displaying the values held in the graph.  These normally shouldn't be called directly, but instead should be accessed using a [/PerfGraphOutput.md] output object.

The `print()` method prints out an indented set of section names and shows their timing like so:

```
-------------------------------------------------------------------------------------------------------------
|                 Section                |   Self(s)  |    %   | Children(s) |    %   |  Total(s)  |    %   |
-------------------------------------------------------------------------------------------------------------
| App                                    |      0.004 |   1.95 |       0.207 |  98.05 |      0.212 | 100.00 |
|   FEProblem::computeUserObjects        |      0.000 |   0.07 |       0.000 |   0.00 |      0.000 |   0.07 |
|   FEProblem::solve                     |      0.014 |   6.59 |       0.119 |  56.44 |      0.133 |  63.03 |
|     FEProblem::computeResidualInternal |      0.000 |   0.01 |       0.079 |  37.45 |      0.079 |  37.45 |
|     FEProblem::computeJacobianInternal |      0.000 |   0.01 |       0.040 |  18.83 |      0.040 |  18.84 |
|     Console::outputStep                |      0.000 |   0.12 |       0.000 |   0.00 |      0.000 |   0.12 |
|   FEProblem::outputStep                |      0.000 |   0.04 |       0.001 |   0.42 |      0.001 |   0.46 |
|     PerfGraphOutput::outputStep        |      0.000 |   0.00 |       0.000 |   0.00 |      0.000 |   0.00 |
|     Console::outputStep                |      0.001 |   0.32 |       0.000 |   0.00 |      0.001 |   0.32 |
|     CSV::outputStep                    |      0.000 |   0.10 |       0.000 |   0.00 |      0.000 |   0.10 |
-------------------------------------------------------------------------------------------------------------
```

`Self` time is the time actually taken by the section while `Children` time is the cumulative time of all of the sub-sections below that section and `Total` is the sum of the two.  The `%` columns represent the percent of the total run-time of the app for the number in the column to the left.

There are also two other ways to print information out about the graph using `printHeaviestBranch()` and `printHeaviestSections()`.  These are described well over on the [/PerfGraphOutput.md] page.