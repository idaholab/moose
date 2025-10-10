# PinTempSolver

!syntax description /Problem/PinTempSolver

## Overview
`PinTempSolver` provides a 1-D, axisymmetric **radial heat-conduction** model for metallic SFR pins that is tightly coupled to the Subchannel coolant solver. It computes the pin temperature distribution at every axial location and returns the **convective wall heat flux** to the subchannel energy equation. Conversely, it receives the **convective boundary condition** (HTC and bulk coolant temperature) from the subchannel solution. The class inherits from `TriSubChannel1PhaseProblem` so it runs inside the existing subchannel outer iteration.

> **Key points**
> - Computes fuel→gap→clad radial temperatures per pin and axial cell.
> - Uses subchannel fields to form the outer Robin BC; optional per-pin jet overrides.
> - Returns **convective heat deposition** to channels (not raw pin power) once a transient begins.
> - Overrides `externalSolve()` so the pin conduction solve executes within each subchannel iteration.

See background in [subchannel_theory.md].

## Governing Equations
The radial energy equation with internal heat generation is
\[
\rho(r,T)c_p(r,T)\frac{\partial T}{\partial t}
= \frac{1}{r}\frac{\partial}{\partial r}\!\left[r k(r,T)\frac{\partial T}{\partial r}\right] + q'''(z),
\]
with \(q'''(z)\) derived from the local linear heat rate and fuel area.

**Boundary/Interface conditions**
- Fuel centerline or inner radius: symmetry (zero flux).
- Fuel–clad gap: thermal contact via effective gap conductance \(h_\text{gap}\).
- Outer cladding surface: Robin BC with \(h_w\) and coolant \(T_\infty\) from the subchannel fields (or from pin jets if specified).

## Numerical Model
A node-centered radial mesh is built across fuel, gap interface, and cladding. Steady and transient systems are assembled into a tridiagonal-like linear system per (pin, axial cell). Transients use an implicit first-order time discretization with iterative linearization to a relative tolerance of \(10^{-5}\).

## Coupling with the Subchannel Solver

### Coolant → Pin (outer BC)
Local wall HTC \(h_w\) is computed from subchannel properties using a pitch/Pe-based Nusselt correlation, with area-fraction weights by subchannel type (center/edge/corner). For pins with jets, per-pin \(h_\text{jet}\) and \(T_\text{jet}\) override the averaged values.

### Pin → Coolant (energy source)
For each channel, the **convective** heat from adjacent pins is summed and supplied to the channel energy equation:
\[
\dot{Q}_\text{conv}=\sum_\text{pins} f_\text{geom}\,\pi D\,h_w\,(T_{\text{clad,o}}-T_\infty).
\]

**Initialization vs. transient behavior**
- At initialization (no prior pin temperatures), channels use **pin linear power** projected to the mesh.
- For \(t>0\), channels use **convective deposition** from the pin wall—improving energy consistency during flow/temperature transients.

## Execution Flow
1. Subchannel iteration updates pressure/flow/thermal properties.
2. `PinTempSolver` initializes on first call (radial mesh, arrays, seeds).
3. Per pin and axial cell: build BCs → solve radial system → update BCs once for consistency.
4. Outputs: pin outer-surface temperature field, channel heat deposition, and optional duct wall temperature.

## Materials
Temperature-dependent properties for **U–Pu–Zr** fuel (conductivity, heat capacity, density with porosity partitioning) and **HT9** cladding are evaluated per node. Inputs for solidus/liquidus/fusion heat exist for future latent-heat extensions.

## Change Summary vs. Legacy
- `externalSolve()` is overridden so the pin conduction solve is included with subchannel solves.
- `computeAddedHeatPin()` now deposits **convective** wall heat (not just pin power) once the transient starts, keeping the coolant energy equation consistent with wall temperatures.

## Example Input File Syntax
!listing /test/tests/problems/SFR/pin_temp_solverSS/pin_tempss.i block=Problem language=moose

## Parameters
!syntax parameters /Problem/PinTempSolver

!syntax inputs /Problem/PinTempSolver

!syntax children /Problem/PinTempSolver
