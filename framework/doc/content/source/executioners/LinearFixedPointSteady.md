# LinearFixedPointSteady

!syntax description /Executioner/LinearFixedPointSteady

## Overview

This executioner iterates between multiple [LinearSystem.md]
objects declared in the same steady state problem. The number of
iterations and tolerances can be defined
[!param](/Executioner/LinearFixedPointSteady/number_of_iterations) and
[!param](/Executioner/LinearFixedPointSteady/absolute_tolerance)
parameters respectively. If the user wants to accept the last
iteration as a converged result, the
[!param](/Executioner/LinearFixedPointSteady/continue_on_max_its)
parameter should be switched to `true`.
The systems are solved in the order they are defined in the
[!param](/Executioner/LinearFixedPointSteady/linear_systems_to_solve)
parameter.

!syntax parameters /Executioner/LinearFixedPointSteady

!syntax inputs /Executioner/LinearFixedPointSteady

!syntax children /Executioner/LinearFixedPointSteady
