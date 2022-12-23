# Incompressible Finite Volume Navier Stokes

MOOSE's Incompressible Navier Stokes Finite Volume (INSFV) implementation uses a
colocated grid. To suppress the checkerboard pattern in the pressure field,
`INSFV` objects support a Rhie-Chow interpolation for the velocity. Users can get
a feel for INSFV by looking at some tests. In addition, to ease the burden of
preparing long input files, the [NavierStokesFV](/Modules/NavierStokesFV/index.md)
action syntax can also be used to set up INSFV simulations.

## Lid Driven Cavity Flow

This example solves the INS equations for mass, momentum, and energy in a closed
cavity. Because there are no inlet or outlet boundaries, one pressure degree of
freedom must be constrained in order to eliminate the nullspace. This is done
using the `FVScalarLagrangeMultiplier` object which implements the
mean-zero pressure approach. The finite element theory of the mean-zero approach
is described
[here](https://github.com/idaholab/large_media/blob/master/framework/scalar_constraint_kernel.pdf). The
finite volume implementation is completed by simply substituting unity for the
test functions.

!listing modules/navier_stokes/test/tests/finite_volume/ins/lid-driven/lid-driven-with-energy.i

## Channel Flow

There are examples of both no-slip and free-slip channel flow. The only
difference between the two inputs is in the boundary condition block. For the
no-slip case, a Dirichlet 0 condition is applied to the tangential velocity (in
this case `u`); this condition is not applied for free-slip. A Dirichlet 0
condition is applied on the pressure at the outlet boundary for both
inputs. This is necessary, in particular for the no-slip case, because if a
Dirichlet condition is not applied for a finite volume variable at a boundary,
then a zero-gradient condition is implicitly applied. This is an invalid
boundary condition for no-slip; in fully developed flow we know that pressure,
while constant across the channel cross-section, decreases in the direction of
flow. (Otherwise without that pressure driving-force, how are you going to drive
flow with the walls slowing you down?)

One may reasonably ask why we implicitly apply a zero normal-gradient condition when
Dirichlet conditions are not applied. This is so that `FVFluxKernels` executed
along a boundary have a value for the field in the neighboring ghost
cell. `FVFluxKernels` are always executed along a boundary if `FVDirichletBCs`
are active; their execution ensures that that the Dirichlet condition is weakly
enforced. When `FVDirichletBCs` are not active, `FVFluxKernels` may still be
forced to execute along a boundary by specifying
`force_boundary_execution = true` in the respective block. Forcing execution of a
`FVFluxKernel` on a boundary when no Dirichlet BC is present, e.g. with an
implicit application of the zero normal-gradient condition, is typically identical to
applying a corresponding `FVFluxBC` because the latter only has access to the
cell center value adjacent to the boundary. By directly using a cell-center value
in a `FVFluxBC` (see for example
[`FVConstantScalarOutflowBC`](/FVConstantScalarOutflowBC.md)), you are implicitly applying
the same zero normal-gradient condition. In the future (see
[MOOSE issue #16169](https://github.com/idaholab/moose/issues/16169)), we hope to be able to apply
more appropriate outflow conditions.

- no slip

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/2d-rc-no-slip.i

- free slip

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/2d-rc.i


## Axisymmetric Channel Flow

Channel flow in axisymmetric coordinates is also implemented. Below is an
example of solving the no-slip problem using an `average` interpolation for the
velocity:

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/cylindrical/2d-average-no-slip.i

The Rhie-Chow interpolation can be used simply by specifying
`velocity_interp_method='rc'` either in the input file or from the command
line. An axisymmetric example with free slip conditions, using the Rhie-Chow
interpolation is shown below:

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/cylindrical/2d-rc-slip.i

## Skewness-correction

The skewness-correction of different variables can be enabled by defining the
`face_interp_method=skewness-corrected` parameter for the INSFVVariables and
selecting it as an option in the advection kernels. It has proven to increase
accuracy on unstructured grids. In case of a skewed 2D triangulation, it
increases the order of the $L^2$ error from $O(h)$ to $O(h^2)$ for velocity, and from
$O(h^{0.5})$ to $O(h)$ for pressure. For an example see:

!listing modules/navier_stokes/test/tests/finite_volume/ins/mms/skew-correction/skewed-vortex.i

## Cell-centered vector field reconstruction using face fluxes

### Weller's method

For a detailed description on the origins and properties of this method, see
[!cite](weller2014non) and [!cite](aguerre2018oscillation). In short, this reconstruction
can be used to obtain cell-center velocities and pressure gradients based on face fluxes and
normal pressure face gradients, respectively. It exhibits a second order convergence ($O(h^2)$)
spatially. The expression for the vector value at the cell centers is the following:

!equation id=wellers-reconstruction
\vec{v}_C = \left(\sum\limits_f^{N_f} S_f\vec{n}_f \otimes \vec{n}_f \right)^{-1} \left(\sum\limits_f^{N_f} \vec{n}_f \phi_f \right)

where $\vec{n_f}$ denotes the surface normal, $S_f$ the surface area and $\phi_f$ the
face flux. Latter can be computed by the face vector values by $\phi_f = (\vec{v}_f \cdot n_f) S_f$.

