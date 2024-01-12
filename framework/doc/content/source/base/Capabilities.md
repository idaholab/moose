# Capabilities

The `Capabilities` system introduces a global registry where features and capabilities of
MOOSE based apps can be registred. These features can have values of type

- **boolean** - Indicating if a feature is enabled, such as the optional *thermochimica* library in the `chemical_reactions` module
- **integer** - Such as for example the size of the ADReal backing store (`ad_size`)
- **string** - Such as the name of the compiler used to build the executable
- **version** - A version number for a library used to build MOOSE (e.g. `petsc=3.20.1`). Version numbers are internally stored as strings and have to follow the pattern of one or more numbers deliminated by periods.

The capabilities system API consists of the following static functions in `Moose::Capabilies::`

- `add(name, value)` to register a capability with a given `name` (string) and a `value` of one of the afforementioned types
- `check(conditons)` to check if the currently registred capability conform to a set of conditions (e.g. `ad_size>50 method!=dbg petsc>=3.15.1`)
- `dump()` to build a string with a JSON dictionary of all currently registered capabilities

Capabilities that are known _not_ to be supported should be explicitly registered as `false`.
