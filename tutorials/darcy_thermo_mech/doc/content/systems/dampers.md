# Damper System

A system to limit the computed change to the solution during each non-linear iteration to prevent
the solver from dramatically alteration of the solution from one step to the next. This may prevent,
for example, the solver from attempting to evaluate non-physical values such as negative
temperature.

!---
