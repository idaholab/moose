# INSElementIntegralEnergyAdvection

!syntax description /Postprocessors/INSElementIntegralEnergyAdvection

This class performs volumetric (global, or by block) integration of the energy advection term defined in
[INSADEnergyAdvection.md], which is defined by

\begin{equation}
\rho c_p \vec u \cdot \nabla T
\end{equation}

where $\rho$ is the density, $c_p$ is the
specific heat capacity,
$\vec u$ is the velocity, and $\nabla T$ is the temperature gradient. AD
instantiations of this object are denoted by
`INSADElementIntegralEnergyAdvection`, hand-coded by
`INSElementIntegralEnergyAdvection`. The former retrieves the density and
specific heat capacity as `ADReal` material properties while the former
retrieves the same properties as `Real`.

This class performs volumetric integration (element-based as opposed to
side-based) of the advection term because the incompressibility constraint is
applied to the energy advection term and the term is not integrated by
parts. Details about the finite element implementation of the energy advection
term can be found on the [INSADEnergyAdvection.md] page.

## Example input syntax

The `INSADElementIntegralEnergyAdvection` instance is used in a global energy
balance calculation in

!listing test/tests/finite_element/ins/energy-conservation/q2q1.i block=Postprocessors

for Q2Q1 elements, e.g. second order velocity and temperature and first order
pressure on `QUAD9` elements and in

!listing test/tests/finite_element/ins/energy-conservation/q1q1.i block=Postprocessors

for Q1Q1 elements, e.g. fist order velocity, pressure, and temperature on
`QUAD4` elements.

!syntax parameters /Postprocessors/INSElementIntegralEnergyAdvection

!syntax inputs /Postprocessors/INSElementIntegralEnergyAdvection

!syntax children /Postprocessors/INSElementIntegralEnergyAdvection
