# Incompressible

MOOSE's Incompressible Navier Stokes Finite Volume (INSFV) implementation uses a
colocated grid. To suppress the checkerboard pattern in the pressure field,
`INSFV` objects support a Rhie-Chow interpolation for the velocity. Users can get
a feel for INSFV by looking at some tests.

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
