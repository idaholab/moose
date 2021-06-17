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

The references are initialized using the `declareValue` and `declareValueByName` methods.
It is possible to indicate how the value is to be computed, with respect to parallelism, by setting
the calculation mode, see [#reporter-modes] for more information. The `declareValue` method
works in conjunction with the `validParams` function to allow the input file to dictate the
name of the data being produced. To use this method first define an input parameter for the
data name.

!listing test/src/reporters/TestReporter.C re=^\s*(?P<content>[^\n]*?)\s*// MooseDocs:data

Then initialize the previously created reference by providing the input parameter name. In this
example the initial value is also supplied.

!listing test/src/reporters/TestReporter.C re=^\s*(?P<content>[^\n]*?)\s*// MooseDocs:producer

The `declareValueByName` method works in the same manner except it does not query the parameter
system for the name, it uses the name supplied to specifying the name directly. As such, this name
cannot be modified without recompiling .

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

The get methods accept a `ReporterName` object, which is simply the combination of the name
of the producing Reporter object and the name of the reporter value. It is possible to request
a `ReporterName` in the `validParams` function of the consumer. For example, in the `validParams`
function a parameter with a type of `ReporterName` is specified.

!listing test/src/reporters/TestReporter.C line=addRequiredParam<ReporterName>("int_reporter"

Then in the initialization list a reference to the desired value is initialized by supplying the
name of the parameter containing the `ReporterValue`.

!listing test/src/reporters/TestReporter.C line=_int(getReporterValue<int>("int_reporter")

In the input file, the ReporterName is provide as follows where "a" is the name of the
producer Reporter object in the `[Reporters]` block of the input file that is producing data
with the name "int", which is the name given to the data within the `declareValue` metho of that
object.

!listing reporters/base/base.i line=int_reporter

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

!table caption=Default operations for the default context that occur for Reporter values depending
               on the modes of production and consumption. The prefix `REPORTER_MODE_` is omitted
               for clarity.
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

The `declareValue` and `declareValueByName` methods allow for non-default context to be defined. For
example, the following line declares a Reporter value to use the gather context object. A list of
available contexts follows the code snippet.

!listing test/src/reporters/TestReporter.C re=^\s*(?P<content>_gather.*?)\s*// MooseDocs:gather
                                           replace=["\n", "", " ", "", ",", ", "]


`ReporterBroadcastContext`\\
Automatically performs an MPI scatter of a specified value on the root processor to all processors.

`ReporterScatterContext`\\
Automatically performs an MPI scatter of a vector of data on the root processor to all processors.

`ReporterGatherContext`\\
Automatically performs an MPI gather to a vector of data on the root processor from all processors.

## Reporter Debug Output

The [ReporterDebugOutput.md] output can be added to output to screen all of the Reporter values that were declared and requested, along with their types, producers, contexts, consumers, and consumer modes. This debug output can also be enabled with the `Debug/show_reporters` parameter.

!syntax list /Reporters objects=True actions=False subsystems=False

!syntax list /Reporters objects=False actions=False subsystems=True

!syntax list /Reporters objects=False actions=True subsystems=False
