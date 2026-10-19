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

Outside the tabulated pressure range, the behaviour is set by `out_of_bounds`:

- `extrapolate` (default): $1/B$ and $1/(B\mu)$ are extrapolated linearly using the first or last
  interval of the table. An exception is thrown (causing the time step to be cut) if the
  extrapolated values become non-positive.
- `clamp`: the end values of the table are used, and the pressure derivatives are zero.

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
