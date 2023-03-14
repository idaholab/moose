# Reporter System

The Reporter system may be considered a generalization of the [Postprocessor](Postprocessors/index.md)
and [VectorPostprocessor](VectorPostprocessors/index.md) systems. Each Reporter
object may declare any number of values with any types. By contrast, post-processors
each declare a single, scalar, `Real` value, and while vector post-processors declare
any number of values, they must all be of type `std::vector<Real>`. Reporters can declare
both scalar and vector data of any type, including complex data and arbitrary
classes/structs. The only requirement on the data type is that the types must
have associated `dataLoad` and `dataStore` specializations (see [DataIO.md]).

The reporter system uses a producer/consumer relationship: reporter objects
"produce" data values, which then may be "consumed" by other objects.

## Producing Reporter Data

As noted above, Reporter objects declare any number of values of any type.
Note that these values are automatically registered as
[restartable](restart_recover.md optional=True). For complex types, data serialization routines
might be needed; see [DataIO.md] for more information.

In the Reporter header file, Reporter values are declared as non-const reference
members of the desired types, for example:

!listing test/include/reporters/TestReporter.h re=^\s*(?P<content>[^\n]*?)\s*// MooseDocs:producer

These references are initialized using the `declareValue` and `declareValueByName` methods.
Note that it is possible to indicate how the value is to be computed, with respect to parallelism, by setting
the calculation mode; see [#reporter-modes] for more information. The `declareValueByName` method
uses the supplied string directly as the value name, while the `declareValue` method
gets the value name from the supplied `ReporterValueName` parameter declared in
`validParams`. For example, in `validParams`,

!listing test/src/reporters/TestReporter.C start=MooseDocs:param_begin end=MooseDocs:param_end include-start=False

Then the Reporter data can be declared using `declareValue`:

!listing test/src/reporters/TestReporter.C re=^\s*(?P<content>[^\n]*?)\s*// MooseDocs:producer

Note that in this example, an initial value is supplied for the data.

The calculation of the value(s) occurs by overriding the `execute` method and updating the values:

!listing test/src/reporters/TestReporter.C start=MooseDocs:execute_begin end=MooseDocs:execute_end include-start=False

## Consuming Reporter Data

Any object that inherits from the `ReporterInterface` may consume a value produced by a Reporter.
Values are retrieved in a similar fashion as declared, but use a constant reference. For example,
values to be consumed should create a reference in the class definition:

!listing test/include/reporters/TestReporter.h re=^\s*(?P<content>[^\n]*?)\s*// MooseDocs:consumer

In the initialization list, the `getReporterValue` or `getReporterValueByName`
method is used to initialize the reference:

!listing test/src/reporters/TestReporter.C re=^\s*(?P<content>[^\n]*?)\s*// MooseDocs:consumer

Similarly to `declareValue` and `declareValueByName`, `getReporterValue` uses
the provided string for the value name, whereas `getReporterValueByName` gets
the value name from the parameter named by the provided string.
In the example above, the following appears in `validParams`:

!listing test/src/reporters/TestReporter.C line=addRequiredParam<ReporterName>("int_reporter"

The get methods accept a `ReporterName` object, which is simply the combination of the name
of the producing Reporter object and the name of the reporter value.
In the input file, the ReporterName is provided as follows, where "a" is the name of the
Reporter object in the `[Reporters]` block of the input file that is producing data
with the name "int", which is the name given to the data within the
`declareValue`/`declareValueByName` method of that object:

!listing reporters/base/base.i line=int_reporter

## Outputting Reporter Data id=reporter-output

Reporter values may be output in two formats: [!ac](CSV) and [!ac](JSON). [!ac](CSV) output
is limited to Reporter values with a type of `Real` or `std::vector<Real>`. [!ac](JSON) output will
work for any type that has a `to_json` function; see [JSONOutput.md] for more details.

## Reporter Context and Modes id=reporter-modes

Reporter values use a context system for performing parallel operations automatically. The default
context allows Reporter values to be produced and consumed in various modes. Depending on the mode
produced/consumed, parallel operations will be performed automatically. The following modes exist for
the default context:

- +REPORTER_MODE_ROOT+: Values exist only on the root processor.
- +REPORTER_MODE_REPLICATED+: Values exist and are identical on all processors.
- +REPORTER_MODE_DISTRIBUTED+: Values exist and are different across processors.

Values can be produced or consumed in any of the prescribed modes. When consumed, the mode of
production is checked against the mode of consumption. [producer-consumer-modes] details the
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
Automatically performs an MPI broadcast of a specified value on the root processor to all processors.

`ReporterScatterContext`\\
Automatically performs an MPI scatter of a vector of data on the root processor to all processors.

`ReporterGatherContext`\\
Automatically performs an MPI gather to a vector of data on the root processor from all processors.

## Reporter Debug Output

The [ReporterDebugOutput.md] output can be added to output to screen all of the Reporter values that were declared and requested, along with their types, producers, contexts, consumers, and consumer modes. This debug output can also be enabled with the `Debug/show_reporters` parameter.

!syntax list /Reporters objects=True actions=False subsystems=False

!syntax list /Reporters objects=False actions=False subsystems=True

!syntax list /Reporters objects=False actions=True subsystems=False
