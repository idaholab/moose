# Continuous Galerkin Finite Element Navier Stokes

The CGFE method has been implemented to solve either the incompressible or
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

The derivation for the RZ weak form can be inspected [here](../../media/jw-peterson-rz-derivation.pdf).

### RZ INS tests

For a sufficiently long channel, flow should be fully developed at the channel
exit and `grad_u * normals` should be equal to zero. If that premise is true,
*and* if the pressure is integrated by parts *and* a natural boundary condition
is "imposed" on the outflow boundary, *then* the pressure at the outflow should
also be zero.

In the RZ channel tests, for a steady simulation the volumetric inflow rate
should be equal to the volumetric outflow rate. The inflow is equal to
.3926991, which corresponds to the exact integral of

\begin{equation}
2 * \pi * \int_0^{0.5} (-4 * r^2 + 1) r \mathrm{d}r
\end{equation}

where $(-4 * r^2 + 1)$ represents the inlet function for the normal velocity
component at the inlet. Below is a summary of the volumetric outflow for different INS
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
for all of the four cases use the same gold files between the two
implementations.

### Stabilized RZ INS tests

Below is a summary of different stabilized INS RZ tests. A natural boundary
condition is used on the outflow in all cases. Note that while a second order
basis is capable of exactly capturing the quadratic character of the inlet flow
function, a first order basis is not. Order in the below table refers to the
velocity order.

| Formulation | Inflow | Outflow |
| ----------- | ------ | ------- |
| AD, Not integrated by parts, first order, SUPG and PSPG | .3599742 | .3384178 |
| Hand-coded, Not integrated by parts, first order, SUPG and PSPG | .3599742 | .3384178 |
| AD, Integrated by parts, first order, SUPG and PSPG | .3599742 | .3599742 |
| Hand-coded, Integrated by parts, first order, SUPG and PSPG | .3599742 | .3599742 |
| AD, Not integrated by parts, second order, SUPG and PSPG | .3926991 | .3756223 |
| Hand-coded, Not integrated by parts, second order, SUPG and PSPG | .3926991 | .3926991 |
| AD, Integrated by parts, second order, SUPG and PSPG | .3926991 | .3926991 |
| Hand-coded, Integrated by parts, second order, SUPG and PSPG | .3926991 | .3926991 |

Notes on the above table:

- All cases which have the incorrect result for the outflow approach the correct solution with mesh
  refinement
- While the second order hand-coded cases achieve the correct result at the base level of mesh
  refinement, the equivalent AD cases do not. (Yes the global outflow variable for the integrated by parts
  case is accurate to the comparison tolerance, but an exodiff executed between the AD and
  hand-coded cases shows the files are different.) Given that the first order cases are identical (we
  just symlink between hand-coded and AD first-order cases in the gold directory), the
  difference has to be due to the inability to calculate second derivatives for vector variables,
  which leads to an inability to include the Laplacian of the velocity in the strong form of the
  momentum residual. For a first order basis, the inability to include the Laplacian terms induces
  no error. However, for the second order basis it does.

### INS Recommendations

- If not computing on a displaced mesh, use the hand-coded INS implementation because it is slightly
  faster and it includes the Laplacian terms in SUPG and PSPG stabilization methods, meaning it is
  completely consistent and will exhibit less error in the finite element solution
- If computing on a displaced mesh, use the AD implementation because it will include derivatives
  with respect to displacements in the Jacobian and the nonlinear solve will be more efficient.
- If using the AD implementation, either run unstabilized with second order basis for the velocity
  and first order basis for the pressure, or if desiring stabilization use a first order basis for
  the velocity variable as it will not introduce any inconsistency. (Recall from above that the
  vector variable, AD implementation cannot currently include Laplacian terms. For a first order
  basis this incurs no error, but for a second order basis it does)
