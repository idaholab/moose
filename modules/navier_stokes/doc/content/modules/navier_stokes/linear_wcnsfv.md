# Weakly Compressible Navier Stokes using the Linear Finite Volume discretization

## Equations

The linear finite volume discretization of the weakly compressible Navier Stokes equations is used
to solve the following equations:

- conservation of momentum
- pressure-correction (see [SIMPLE.md])
- turbulence equations
- conservation of energy
- conservation of advected passive scalars
- conservation of an advected phase in a homogeneous mixture

We refer the reader to the respective `Physics` pages, listed in [linear_wcnsfv.md#syntax], for the strong form of the equations.

## Solver algorithm(s)

For steady state simulations, you may use the [SIMPLE.md] executioner which implements the SIMPLE algorithm [!citep](patankar1983calculation).

For transient simulations, you may use the [PIMPLE.md] executioner which implements the PIMPLE algorithm [!citep](greenshieldsweller2022).

## Discretization

### General

We use the linear finite volume discretization, a face-centered finite volume discretization. We have implemented orthogonal
gradient correction and skewness correction for face values, and thus can reach second-order accuracy in many cases.

!alert note
Triangular and tetrahedral meshes currently only achieve first order convergence rates at the moment.

!alert note
This implementation does not require forming a Jacobian because it is solving using the SIMPLE/PIMPLE algorithm, which
involve segregated linear equation solved nested in a fixed point iteration loop, rather than a Newton method-based solver.
The discretization of the equation is optimized to form a right hand side (RHS) and sparse matrices.
Additional details about the linear finite volume discretization can be found on [this page](linear_fv_design.md).

### Advection term

The advection term is discretized using the Rhie Chow interpolation for the face velocities. Additional details may be found in the documentation
for the object handling the computation of the Rhie Chow velocities: the [RhieChowMassFlux.md].

## Syntax id=syntax

These equations can be created in MOOSE using the [LinearFVKernels](syntax/LinearFVKernels/index.md) and [LinearFVBCs](syntax/LinearFVBCs/index.md)
classes, or using the [Physics](syntax/Physics/index.md) classes.
For `LinearWCNSFV`, the relevant `Physics` classes are:

- [WCNSLinearFVFlowPhysics.md] for the velocity-pressure coupling.
- [WCNSLinearFVFluidHeatTransferPhysics.md] for the fluid energy conservation equation.
- [WCNSLinearFVScalarTransportPhysics.md] for the advection of passive scalars.

For `LinearWCNSFV2P`, the relevant `Physics` classes are:

- [WCNSLinearFVTwoPhaseMixturePhysics.md] for a basic implementation of a mixture model.

## Validation

The linear finite volume discretization is being verified and validated as part of the `OpenPronghorn` open-source software.
Please refer to [OpenPronghorn](https://mooseframework.inl.gov/open_pronghorn/) for this ongoing effort.

## Gallery

!alert construction
The gallery has not been created for this discretization yet.
Please refer to [OpenPronghorn](https://mooseframework.inl.gov/open_pronghorn/) for example simulations.
