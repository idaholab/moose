# DataIO

The DataIO.h/.C files contain the declaration and definitions for MOOSE's templated `dataLoad/dataStore` methods. These methods
are scattered around the framework, modules and user applications and are used to store stateful information that cannot be
recalculated during a restore operation. These methods enable MOOSE's checkpointing and restarting operations which enable
several key capabilities in the MOOSE framework including:

- Checkpointing -> the ability to terminate an application and restart it where you left off (useful for batch cluster systems).
- Picard Iteration -> The ability to converge a "tightly" coupled multiApp simulation.
- Restart -> the ability to save stateful data for a [restart](restart_recover.md optional=True) type simulation when using checkpoint format

## What is stateful data?

Stateful data is any value, container of values, or complex data structure that cannont be recomputed from other available
information such as coupled values or field variables. Additionally, it's not any data that is not directly owned by your object.

Here are a couple of examples to consider before defining a dataLoad/dataStore routine:

- If you have an object that produces a value based on an "old" value of a coupled variable, your object does +not+
  contain any stateful data because it is a simple calculation based on a piece of information not owned by your object. No action is
  necessary for your object to be "restartable".
- If your object has a data structure consisting of a vector of pairs of IDs and Real numbers that are computed from a Material
  property, your object does +not+ contain any stateful data and no action is necessary for your object to be "restartable".
- If your object has a simple Boolean used to indicate whether or not you have calculated some quantity before, that you set
  when you run some routine. Your object +does+ contain stateful information since the state of that value depends on internal
  logic in your object. For this scenario, you will need to ask Moose to store your Boolean as "restartableData". See
  [#declareRestartableData].
- If your object contains a dataStructure of some custom type that you produce internally and retrieve existing values from
  over the coarse of the simulation, you have a stateful data and may need to define the dataLoad and dataStore functions
  in your object. See [#declareRestartableData], and [#dataStore_dataLoad].


## declareRestartableData id=declareRestartableData

The declareRestartableData method is used to tell MOOSE that you would like to save some part of your object in a [Backup](Backup.md)
object. This method is templated and declared here:

!listing framework/include/restart/Restartable.h
  re=([^\n]+\n)*[^\n]+declareRestartableData[^\n]*;

This method is templated, so MOOSE will return a reference to the type that you request and manage the data storage for you. For
all built-in types and combinations of containers and built-in types. This is all that needs to be done. If your type or
your container of types is custom, you will have to define the dataLoad and dataStore routines to tell MOOSE how to serialize
your new type.

## dataStore/dataLoad routines id=dataStore_dataLoad

If any object has requested a restartable piece of data that contains or is a custom type, both the dataStore and dataLoad will
need to be defined. These functions describe how to serialize a custom type.

The declarations for the two methods that may need to be specialized for your application take on on a form similar to this:

!listing framework/include/restart/DataIO.h
  re=^[^\n]*Global Load Declarations.*?\n\n

!listing framework/include/restart/DataIO.h
  re=^[^\n]*Global Store Declarations.*?\n\n

### Example

Typically, the serialization routine can be
defined in terms of serializing the individual fields in your custom type. For example. If you had a class `Foo` that contained a
few plain old data types, you'd just define the load and store terms in terms of the combination of those POD types in order.

```language=c++
class Foo
{
  int bar;
  std::string baz;
  std::vector<unsigned int> qux;
};


// Definition
template <>
void
dataStore(std::ostream & stream, Foo & foo, void * context)
{
  // Defined in terms of the simple types that MOOSE already knows how to store
  storeHelper(stream, foo.bar, context);
  storeHelper(stream, foo.baz, context);
  storeHelper(stream, foo.qux, context);
}

template <>
void
dataLoad(std::istream & stream, Foo & foo, void * context)
{
  // Defined in terms of the simple types that MOOSE already knows how to read.
  // Note the order of the calls, they should match the dataStore routine since each
  // type is being read from the stream.
  loadHelper(stream, foo.bar, context);
  loadHelper(stream, foo.baz, context);
  loadHelper(stream, foo.qux, context);
}
```
