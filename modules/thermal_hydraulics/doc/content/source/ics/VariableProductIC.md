# VariableProductIC

!syntax description /ICs/VariableProductIC

The value of the other variables in the product is naturally equal to their initial conditions,
as initial conditions are only run once at the beginning of the simulation.

This is typically used to set the initial condition for `rho*A`, the specific density and `rho*u*A`, the specific
momentum.

!alert note
For `rho*A` or `rho*u*A`, this initial condition is usually added to the `Simulation` by the `FlowModel`, depending on the parameters
passed to each [Component](syntax/Components/index.md).

!syntax parameters /ICs/VariableProductIC

!syntax inputs /ICs/VariableProductIC

!syntax children /ICs/VariableProductIC
