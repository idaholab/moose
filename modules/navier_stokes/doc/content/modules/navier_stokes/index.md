# Navier-Stokes Module

The MOOSE Navier-Stokes module is a library for the implementation of simulation tools that solve the
Navier-Stokes equations using the continuous Galerkin finite element (CGFE) method. The Navier-Stokes
equations are usually solved using either the pressure-based, incompressible formulation (assuming a
constant fluid density), or the density-based, compressible formulation.

[The CGFE method](navier_stokes/cgfe.md) has been implemented to solve either the incompressible or
compressible Navier-Stokes equations. The original CGFE method is usually not numerically stable for
solving problems when the Peclet number is greater than 2. An SUPG (Streamline Upwind Petrov
Galerkin) scheme is implemented for stabilized solution in smooth compressible flows. A
low-diffusion, discontinuity/shock-capturing scheme is required but currently absent for the CGFE
method to obtain non-oscillatory solutions of flow problems that contain contact discontinuity or
shock waves. For compressible flow problems, users can choose the CGFE method only when the flow
field is sufficiently smooth.

For incompressible flow, we have implemented both pressure-stabilized
Petrov-Galerkin (PSPG) and streamline-upwind Petrov-Galerkin (SUPG) methods. The
former stabilization allows use of equal order shape functions by introducing an
on-diagonal dependence in the pressure equation, removing the saddle-point
nature of the problem. The latter SUPG method allows
simulation at much higher Reynolds numbers than if SUPG was not used. For an
overview of the incompressible Navier-Stokes capability, please see the journal
article
[here](https://www.sciencedirect.com/science/article/pii/S0965997817310591?via%3Dihub)
or the pre-print [here](https://arxiv.org/pdf/1710.08898.pdf). Note that
automatic differentiation versions of the incompressible objects have been
created; these objects are currently not as performant as their hand-coded
peers, but they can be used seamlessly in simulations with mesh deformation and
are guaranteed to generate correct Jacobians.

Known limitations of AD INS implementation (these limitations are not present
with the hand-coded INS implementation):

- Stabilization methods will be inconsistent if a second order `LAGRANGE_VEC` basis is used
  for the velocity variable. This is because second derivatives are not
  implemented in libMesh for vector `FE` types, and consequently we cannot add in the contribution
  from the viscous term which includes a Laplacian operation
- Only the Laplace form of the viscous term is implemented, e.g. there is
  currently no support for the traction form.

## Notes on INS RZ

Notes regarding INS RZ equations derivation:

- Two additional terms are introduced into the r-component of the strong
  representation of the momentum equation viscous term
- One additional term is introduced into the r-component of the weak
  representation of the momentum equation viscous term
- One additional term is introduced into the z-component of the strong
  representation of the momentum equation viscous term
- Zero additional terms are introduced into the z-component of the weak
  representation of the momentum equation viscous term
- An additional pressure term will enter the weak form of the r-component of the
  momentum equation if the pressure term was integrated by parts. No additional
  terms appear in the strong form if the term is integrated by parts
  (integration by parts is a part of forming the weak form)
- An additional term is introduced into the mass balance equation

The derivation is given in a document prepared by John Peterson, to be linked to
from here soon.

### RZ INS tests

For a sufficiently long channel, flow should be fully developed at the channel
exit and `grad_u * normals` should be equal to zero. If that premise is true,
*and* if the pressure is integrated by parts *and* a natural boundary condition
is "imposed" on the outflow boundary, *then* the pressure at the outflow should
also be zero.

In the RZ channel tests, for a steady simulation the volumetric inflow rate
should be equal to the volumetric outflow rate. The inflow is equal to
.3926991. Here is a summary of the outflow numbers for different INS
formulations:

| Formulation | Outflow |
| ----------- | ------- |
| Not integrated by parts, natural BC | .3926991 |
| Integrated by parts, natural BC | .3926991 |
| Not integrated by parts, NoBCBC | .3926993 |
| Integrated by parts, NoBCBC | .3926993 |

For the NoBCBC cases, if `Mesh/uniform_refine=2` is applied, then the outflow
converges to the correct solution of .3926991. Note that the results in the above table are achieved
whether using the standard variable, hand-coded Jacobian INS implementation or
the vector variable, AD Jacobian INS implementation. In fact the steady tests
for all of the four cases use the same gold files between the two implementations.

!syntax complete groups=NavierStokesApp
