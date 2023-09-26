# Step 3 - Adding additional terms to the heat equation

In the previous step, we modified boundary conditions to solve a meaningful
problem that provided the steady-state solution to the heat equation
by neglecting time-dependent terms. In this step, we will add terms to the 
heat equation by adding kernels. The heat equation with the full set of terms
typically used for heat transfer within a solid or stationary fluid is:

\begin{equation}\label{eq:heat_equation_tutorial}
  \rho(t, \vec{x}) c(t, \vec{x})\frac{\partial T}{\partial t} = \nabla k(t,\vec{x}) \nabla T + \dot{q} ~\text{for}~\vec{x} \in \Omega,
\end{equation}

where $T$ is temperature, $t$ is time, $\vec{x}$ is the vector of spatial coordinatess, $\rho$ is the density, $c$ is the specific heat capacity, $k$ is the thermal conductivity, $\dot{q}$ is a heat source, and $\Omega$ is the solution domain.

The first step will be to add the time derivative term to this equation, and
then in a subsequent step, a volumetric heating term will be added.

!listing modules/heat_conduction/tutorials/introduction/therm_step03.i

## Input file

### `Variables`

The first change necessary to convert this from a quasi-steady-state problem to
a transient problem is to define a reasonable initial condition for the
temperature variable. The default initial value of variables is 0, which did
not affect the steady-state solution in the previous step, but has a signficant
effect on the transient solution. The `initial_condition` parameter is set
for the `T` variable to 300. Units of Kelvin are used here, and this is close
to room temperature.

!listing modules/heat_conduction/tutorials/introduction/therm_step03.i block=Variables

### `Kernels`

An additional block is added under the `Kernels` top level block to include the
time dependent term $\rho(t, \vec{x}) c(t, \vec{x})\frac{\partial T}{\partial t}$
in the heat equation. The Kernel that provides this term is named `HeatConductionTimeDerivative`
because this term has a dependence on the time derivative of the temperature.

!listing modules/heat_conduction/tutorials/introduction/therm_step03.i block=Kernels

### `Materials`

The `HeatConductionTimeDerivative` Kernel depends on two additional material
properties: `density` and `specific_heat`, which must be provided by material
models. The specific heat can be provided by adding an additional parameter
to the `HeatConductionMaterial` Material block.

The density is not provided by `HeatConductionMaterial` because that is a property
that is used by other physics models, in particular by the models for solid
mechanics. There is a specialized model to compute the density of a deforming
material that is commonly used in those calculations, but there is no deformation
in the present simulation, so a constant property can be used. The `GenericConstantMaterial`
is a material model that can be used to define one or more properties by providing
the name of the property and the constant value. This is used here as a simple way
to define the density.

!listing modules/heat_conduction/tutorials/introduction/therm_step03.i block=Materials

## Questions

Before running the model, consider the initial conditions and boundary conditions,
and estimate what expected solution should be when the time-dependent term
is included.

[Click here for the answer.](heat_conduction/tutorials/introduction/answer03a.md)

Now go ahead and run the input and visualize the result to see if it matches
the behavior you would expect.

### Exploring parameters

Which parameters would you expect to be able to change to give a solution that matches
more closely with the steady-state solution? Try changing those parameters to see if they
have the expected effect after re-running the model.

[Click here for the answer.](heat_conduction/tutorials/introduction/answer03b.md)

### Sidebar: 

> Some problems also have volumetric heating, which can be included by adding another
> Kernel.

[Click here for the sidebar on adding volumetric heating.](heat_conduction/tutorials/introduction/therm_step03a.md)


### Comparison of results

A Python script called `therm_soln_compare.py` in this directory is set up to
plot the results of the problems from steps 2a, 3, and 3a to show a side-by-side
comparison of the temperature profiles at the end of the analyis for these
cases. This script depends on having the `matplotlib` and `pandas` libraries
available in your Python installation. It is executed by typing:

```
./therm_soln_compare.py
```

from the directory where the problems are run. It will generate an image called
`therm_soln_compare.png` with line plots for these three cases.
