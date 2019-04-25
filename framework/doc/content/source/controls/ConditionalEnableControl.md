# ConditionalEnableControl

`Control`s deriving from `ConditionalEnableControl` allow MOOSE objects to be enabled or
disabled according to some condition. Nearly all types of MOOSE objects (`Kernel`, `BC`, etc.)
have a parameter `enable` that is controllable.

Two list parameters exist: `enable_objects` and `disable_objects`. The former
is used to specify which objects should be enabled when the specified condition
is met, and the latter is used to specify which objects should be disabled when
the specified condition is met. See the
[Object and Parameter Names](syntax/Controls/index.md#object-and-parameter-names)
section for the syntax of describing object names.

If the parameter `reverse_on_false` is set to true (as it is by default),
the objects in the `enable_objects` list are disabled when the condition is
false, and the objects in the `disable_objects` list are enabled when the
condition is false.
