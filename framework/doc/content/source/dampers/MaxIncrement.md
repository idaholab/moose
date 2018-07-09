# MaxIncrement

!syntax description /Dampers/MaxIncrement

The `MaxIncrement` damper limits the change of a variable from one nonlinear
iteration to the following iteration. A smaller value set as the `max_increment`
will results in more nonlinear steps.

## Example Input Syntax

!listing test/tests/dampers/max_increment/max_increment_damper_test.i block=Dampers

!syntax parameters /Dampers/MaxIncrement

!syntax inputs /Dampers/MaxIncrement

!syntax children /Dampers/MaxIncrement
