# HydrostaticPressureFunction

This function computes the hydrostatic pressure distribution in a continuous
fluid, given a reference point and pressure value:

!equation
p(\mathbf{r}) = p_\text{ref} - \rho_\text{ref} \mathbf{g} (\mathbf{r}_\text{ref} - \mathbf{r})

where

- $\mathbf{r}_\text{ref}$ is the reference point,
- $p_\text{ref}$ is the pressure at that point,
- $T_\text{ref}$ is the temperature at that point,
- $\rho_\text{ref} = \rho(p_\text{ref}, T_\text{ref})$ is the reference density, and
- $\mathbf{g}$ is the gravity acceleration vector.

!alert warning
Note that this formulation is only exact if the density does not change between
$\mathbf{r}_\text{ref}$ and $\mathbf{r}$.

!syntax parameters /Functions/HydrostaticPressureFunction

!syntax inputs /Functions/HydrostaticPressureFunction

!syntax children /Functions/HydrostaticPressureFunction
