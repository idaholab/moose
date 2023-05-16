# SolutionTimeAdaptiveDT

!syntax description /Executioner/TimeSteppers/SolutionTimeAdaptiveDT

The idea is to find the simulation time step such as the computational cost of a step is the lowest.
The computational cost of a time step solve depends on many factors, this time stepper studies the
time step dependence.

This time stepper tries to decrease the simulation time in every time step by reacting
to changes in computational time. If the computational cost of a time step increases two steps
in a row, then it will either lower or increase the time step, depending on what it did for those
two previous steps. For example, if it increased the time steps twice in a row and the solve time
increased, then it will attempt to decrease the time steps, for at least two steps, until the
dynamic is reversed.

When changing the time step, it is modified using this equation

!equation
dt^{n+1}= dt^n ( 1 + \text{percent change} * \pm 1)

The sign of the update is chosen as explained above.

## Example input syntax

!listing test/tests/executioners/executioner/sln-time-adapt.i block=Executioner

!syntax parameters /Executioner/TimeSteppers/SolutionTimeAdaptiveDT

!syntax inputs /Executioner/TimeSteppers/SolutionTimeAdaptiveDT

!syntax children /Executioner/TimeSteppers/SolutionTimeAdaptiveDT
