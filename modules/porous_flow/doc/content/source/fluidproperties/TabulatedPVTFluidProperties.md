# TabulatedPVTFluidProperties

!syntax description /FluidProperties/TabulatedPVTFluidProperties

## Overview

`TabulatedPVTFluidProperties` provides isothermal fluid properties from a table of
pressure $p$, formation volume factor $B$ and dynamic viscosity $\mu$. This is the format used
for dead oil (`PVDO`) and dry gas (`PVDG`) tables in black-oil reservoir simulators, so
existing PVT data can be used directly in PorousFlow.

The formation volume factor is the ratio of the volume of the fluid at reservoir conditions to
its volume at standard (surface) conditions. The density is therefore

\begin{equation}
\rho(p) = \frac{\rho_{sc}}{B(p)},
\end{equation}

where $\rho_{sc}$ is the density at standard conditions (`density_sc`).

## Interpolation

Following the convention of black-oil simulators, the quantities $1/B$ and $1/(B\mu)$ are
interpolated linearly in pressure, rather than $B$ and $\mu$ themselves. These quantities are
proportional to the density and to the mobility of the fluid, and they vary much more linearly
with pressure than $B$ does, particularly for gases. The viscosity is recovered as

\begin{equation}
\mu(p) = \frac{1/B}{1/(B\mu)}.
\end{equation}

The derivatives of density and viscosity with respect to pressure are computed exactly from the
piecewise-linear interpolants, so they can be used in the Jacobian.

The interpolation uses the MOOSE `LinearInterpolation` utility.

Outside the tabulated pressure range, the behavior is set by `out_of_bounds_behavior`, which has
the same options as [TabulatedFluidProperties](TabulatedFluidProperties.md):

- `throw` (default): an exception is thrown, so the time step is cut. This ensures that no results
  are computed with properties outside the range of the PVT data.
- `set_to_closest_bound`: the pressure is moved to the closest bound of the table, so the end values
  of the table are used and the pressure derivatives are zero.
- `declare_invalid`: as `set_to_closest_bound`, but the solution is also marked as invalid.
- `warn_invalid`: as `set_to_closest_bound`, but a solution warning is also recorded.
- `ignore`: $1/B$ and $1/(B\mu)$ are extrapolated linearly using the first or last interval of the
  table. An exception is still thrown if the extrapolated values become non-positive.

!alert warning
With any option other than `throw`, results in parts of the domain where the pressure lies outside
the table should be used with care: the fluid properties there are not supported by the PVT data.
Where possible, extend the table to cover the expected pressure range instead.

## Thermal properties

The table contains no temperature dependence, so the density and viscosity are independent of
temperature. For use in non-isothermal simulations, a constant specific heat capacity at constant
volume $c_v$ is used:

\begin{equation}
e = c_v T, \qquad h = e + \frac{p}{\rho}.
\end{equation}

## Example input syntax

!listing modules/porous_flow/test/tests/fluids/tabulated_pvt.i block=FluidProperties

The object is used with [PorousFlowSingleComponentFluid](PorousFlowSingleComponentFluid.md), so it
can be used for one phase of a two-phase oil-water simulation with, for example,
`PorousFlow2PhasePS`.

!syntax parameters /FluidProperties/TabulatedPVTFluidProperties

!syntax inputs /FluidProperties/TabulatedPVTFluidProperties

!syntax children /FluidProperties/TabulatedPVTFluidProperties
