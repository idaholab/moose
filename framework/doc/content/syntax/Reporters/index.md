# Reporter System

A Reporter object in MOOSE is a C++ object that computes a single value with an arbitrary type. The
value computed is some sort of aggregation of data from a simulation. For example, the [MeshInfo.md]
object reports various quantities regarding the mesh such as the number of nodes, elements, and
degrees-of-freedom.

The system relies on a producer/consumer relationship. The Reporter object produces any number of
aggregation values and other objects consume these values.

## Producer (Reporter objects)

The Reporter system builds from MOOSE's [UserObject](/UserObjects/index.md) system. Each Reporter
contains the full interface of a UserObject but is also expected to declare one or more values that
to be produced by the Reporter object. Generally, there are no restrictions on the supported types for
the Reporter values. All values declared are automatically registered as
[restartable](restart_recover.md optional=True). For complex types data serialization routines
might be needed, see [#reporter-value-types].

Values to be computed are stored via a reference to the desired type, for example:

!listing test/include/reporters/TestReporter.h re=^\s*(?P<content>[^\n]*?)\s*// MooseDocs:producer

The references are initialized using the `declareValue` method as follows. It is possible to
indicate how the value is to be computed, with respect to parallelism, by setting the calculation
mode, see [#reporter-modes] for more information.

!listing test/src/reporters/TestReporter.C re=^\s*(?P<content>[^\n]*?)\s*// MooseDocs:producer

The calculation of the value(s) occurs by overriding the `execute` method and updating the values
for references.

!listing test/src/reporters/TestReporter.C start=MooseDocs:execute_begin end=MooseDocs:execute_end include-start=False

## Consumer (other objects)

Any object that inherits from the `ReporterInterface` may consume a value produced by a Reporter.
Values are retrieved in a similar fashion as declared, but use a constant reference. For example,
values to be consumed should create a reference in the class definition.

!listing test/include/reporters/TestReporter.h re=^\s*(?P<content>[^\n]*?)\s*// MooseDocs:consumer

In the class declaration the `getReporterValue` method is used to initialize the reference.

!listing test/src/reporters/TestReporter.C re=^\s*(?P<content>[^\n]*?)\s*// MooseDocs:consumer

The `getReporterValue` method automatically checks for an input parameter by the given name and the
name specified in the parameter will be used. To retrieve a Reporter value directly by name use
the `getReporterValueByName` method.


## Reporter Value Types id=reporter-value-types

As stated above, Reporter values may be of an type. This includes arbitrary classes or structs.
However, the types must have an associated `dataLoad` and `dataStore` function specialization,
please refer to [DataIO.md] for more information on these functions.

## Reporter Output id=reporter-output

Reporter values are outputted in two forms [!ac](CSV) or [!ac](JSON) files. [!ac](CSV) output
is limited to Reporter values with a type of `Real` or `std::vector<Real>`. [!ac](JSON) output will
work for arbitrary types, if the type has a `to_json` function, see [JSONOutput.md] for more details.

## Reporter Context and Modes id=reporter-modes

Reporter values use a context system for performing parallel operations automatically. The default
context allows Reporter values to be produced and consumed in various modes. Depending on the mode
produced/consumed parallel operations will be performed automatically. The following modes exist for
the default context.

- +REPORTER_MODE_ROOT+: Values exist only on the root processor.
- +REPORTER_MODE_REPLICATED+: Values exist and are identical on all processors.
- +REPORTER_MODE_DISTRIBUTED+: Values exist and are different across processors.

Values can be computed or consumed in any of the prescribed modes. When consumed the mode of
production is checked against the mode consumption. [producer-consumer-modes] details the
actions taken by the various possible modes of production and consumption for a Reporter value.

!table caption=Default operations for the default context that occur for Reporter values depending on the modes of production and
               consumption. The prefix `REPORTER_MODE_` is omitted for clarity.
       id=producer-consumer-modes
| Producer Mode | Consumer Mode | Operation |
| :- | :- | :- |
| ROOT | ROOT | Do nothing |
| REPLICATED | ROOT | Do nothing |
| REPLICATED | REPLICATED | Do nothing |
| DISTRIBUTED | DISTRIBUTED | Do nothing |
| ROOT | REPLICATED | MPI Broadcast |
| ROOT | DISTRIBUTED | Error |
| REPLICATED | DISTRIBUTED | Error |
| DISTRIBUTED | ROOT | Error |
| DISTRIBUTED | REPLICATED | Error |

The `declareValue` method allows for non-default context to be defined. For example, the following
line declares a Reporter value to use the gather context object. A list of available contexts
follows the code snippet.

!listing test/src/reporters/TestReporter.C re=^\s*(?P<content>[^\n]*?)\s*// MooseDocs:gather

`ReporterBroadcastContext`\\
Automatically performs an MPI scatter of a specified value on the root processor to all processors.

`ReporterScatterContext`\\
Automatically performs an MPI scatter of a vector of data on the root processor to all processors.

`ReporterGatherContext`\\
Automatically performs an MPI gather to a vector of data on the root processor from all processors.


!syntax list /Reporters objects=True actions=False subsystems=False

!syntax list /Reporters objects=False actions=False subsystems=True

!syntax list /Reporters objects=False actions=True subsystems=False
