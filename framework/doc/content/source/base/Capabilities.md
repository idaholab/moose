# Capabilities

The `Capabilities` system introduces a global registry where features and capabilities of
MOOSE based apps can be registered. These features can have values of type

- **boolean** - Indicating if a feature is enabled, such as the optional *thermochimica* library in the `chemical_reactions` module
- **integer** - Such as for example the size of the ADReal backing store (`ad_size`)
- **string** - Such as the name of the compiler used to build the executable
- **version** - A version number for a library used to build MOOSE (e.g. `petsc>=3.20.1`). Version numbers are internally stored as strings and have to follow the pattern of one or more numbers delimitated by periods.

The capabilities system API consists of the following static functions in `Moose::Capabilities::`

- `add(name, value)` to register a capability with a given `name` (string) and a `value` of one of the aforementioned types
- `check(conditions)` to check if the currently registered capability conform to a set of conditions (e.g. `ad_size>50 & method!=dbg & petsc>=3.15.1`)
- `dump()` to build a string with a JSON dictionary of all currently registered capabilities

Capabilities that are known _not_ to be supported should be explicitly registered as `false`.

## Command line arguments

`--show-capabilities` produces a JSON dump of the registered capabilities for the current application. If an input file is specified, capabilities from any dynamically registered modules or apps are included in the dump.

`--required-capabilities="..."` performs a check against the supplied conditional. This option is for internal use by the test harness. If the required capabilities are not fulfilled the application exists with code `77`, indicating to the test harness that the test should be skipped.

## Test harness

The `capabilities = '...'` parameter is available in `tests` spec files to enable/disable tests conditionally based on the required and available capabilities of the application. The `capabilities` parameter replaces the multitude of separate tests spec options for individual requirements (for example `exodus = true` and `exodus_version = >8.0` would be replaced by `capabilities = 'exodus>8.0'`).

Capabilities conditionals can use comparison operators (`>`,`<`,`>=`,`<=`,`=!`,`=`), where the name of the capability must always be on the left hand side. Comparisons can be performed on strings `compiler!=GCC` (which are case insensitive), integer numbers `ad_size>=50`, and version numbers `petsc>3.8.0`. The state of a boolean valued capability can be tested by just specifying the capability name `chaco`. This check can be inverted using the `!` operator `!chaco`.

The logic operators +and+ `&` and +or+ `|` can be used to chain multiple checks `thermochimica & thermochimica>1.0`. Parenthesis `(` `)` can be used to build complex logic expressions.

The test harness performs a two tiered check of the capabilities. The first tier is a static check against the capabilities exported from the base application (using `--show-capabilities`). Based on this check a decision can be made on whether to skip a test, if all capabilities that are being tested are registered by the base application. If a check is made against an unregistered capability the second tier check is invoked.

In the second tier the application is executed with the `--required-capabilities` parameter. This check is only done when `dynamic_capabilities = true` is set within the test specification (the default is false). In this check an `[Application]` block in the current input could dynamically load modules or apps that register new capabilities not yet known by the base application.
