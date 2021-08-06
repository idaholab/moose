# Finite Volume Incompressible Porous media Navier Stokes

## Equations

This module implements the porous media Navier Stokes equations. They are expressed in terms of the superficial
viscosity $\vec{v}_d = \eps \vec{V}$ where $\eps$ is the porosity and $\vec{V}$ the interstitial velocity. The
superficial viscosity is also known as the extrinsic or Darcy velocity. The other non-linear variables used are
pressure and temperature. This is known as the primitive superficial set of variables.

Mass equation:
\begin{equation}
\div (\rho \vec{v}_d = 0)
\end{equation}

Momentum equation, with friction and gravity force as example forces:
\begin{equation}
\dfrac{\partial \rho \mathbf{v}_D}{\partial t} + \nabla \cdot (\dfrac{\rho}{\epsilon} \mathbf{v}_D \otimes \mathbf{v}_D) &= \nabla \cdot (\mu \nabla \dfrac{\mathbf{v}_D}{\epsilon}) - \epsilon \nabla p + \epsilon (\mathbf{F}_g + \mathbf{F}_f)
\end{equation}

Fluid phase energy equation, with a convective heat transfer term:
\begin{equation}
\dfrac{\partial \epsilon \rho c_p T_f}{\partial t} + \nabla \cdot (\dfrac{\rho}{\epsilon} \mathbf{v}_D \rho c_{pf} T_f) &= \nabla \cdot (\kappa_f \nabla T_f) - \epsilon \alpha (T_f - T_s)
\end{equation}

Solid phase energy equation, with convective heat transfer and an energy source $\dot{Q}$:
\begin{equation}
\dfrac{\partial (1-\epsilon) \rho c_{ps} T_s}{\partial t} &= \nabla \cdot (\kappa_s \nabla T_s) + (1-\epsilon) \alpha (T_f - T_s) + (1-\epsilon) \dot{Q}
\end{equation}

where $\rho$ is the fluid density, $\mu$ the viscosity, $c_p$ the specific heat capacity $\alpha$ the convective heat transfer coefficient.

## Implementation for a straight channel

We define a straight channel using a simple Cartesian mesh.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=Mesh

The non linear variables are specified among the `INSFV` and `PINSFV` sets of variables as their evaluation
and the computation of their gradients on faces is specific to the treatment of the Navier Stokes equations.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=Variables

The porous media Navier Stokes equation are implemented by using `FVKernels` which correspond to
each term of the equations. In the following example, the first kernel is the mass advection kernel.
This kernel corresponds to the conservation of mass. It acts on the pressure non-linear variable which
appears in the mass equation through the Rhie Chow interpolation of the mass fluxes on the element faces, and
through its action on the velocity in the momentum equation.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=FVKernels/mass

Then we add the kernels acting on the 'x' component of the momentum, so essentially transcribing the first
component of the momentum equation into MOOSE.

The momentum advection term is defined by a `PINSFVMomentumAdvection` kernel.
The momentum equation requires a velocity and an advected quantity interpolation method. This is because this
kernel is a `FVFlux` kernel, it uses the divergence theorem to compute the divergence based on face fluxes rather
than on the volumetric divergence value. The velocity interpolation method may be `average` or `rc` (Rhie Chow).
The latter is preferred to avoid oscillations in pressure due to the grid collocation.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=FVKernels/u_advection

The next term is the momentum diffusion. This term arises from viscous stress in the fluid. The incompressible
approximation simplifies its treatment. This term is also evaluated using the divergence theorem.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=FVKernels/u_viscosity

The pressure gradient term in the momentum equation is expressed using a `PINSFVMomentumPressure` term.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=FVKernels/u_pressure

The same set of kernels are defined for the 'y' component of the superficial velocity to transcribe the 'y'
component of the momentum equation.

This test features a variety of boundary conditions listed in the `FVBCs` block. They may not all be used at
the same time, the user has to selectively activate the desired one. We list the following example boundary conditions,
for the first component of the superficial velocity:

- no slip walls
  !listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=FVBCs/no-slip-u

- free slip walls
  !listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=FVBCs/free-slip-u

- symmetry axis. This symmetry condition should also be indicated for the pressure variable.
  !listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=FVBCs/symmetry-u
  !listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=FVBCs/symmetry-p

- inlet velocity, to specify mass flux given that density is constant
  !listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=FVBCs/inlet-u

- momentum advection outflow (only for a mean-pressure approach, equivalent to executing the momentum advection kernel on the boundary)
  !listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=FVBCs/outlet-u


!alert note
If the PINSFV version of a boundary condition does not exist, it may be because the INSFV version is valid to use,
replacing velocity by superficial velocity.

The pressure boundary condition is usually only set at the outlet:
!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=FVBCs/outlet-p

For a mean-pressure approach, usually for cavity problems, the user may specify a mass advection boundary condition. This
is equivalent to executing the mass advection kernel on boundaries.
!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc.i block=FVBCs/outlet-p-novalue

## Example inputs : heated straight channel

This channel features a porous media with temperature exchanged by convection between the fluid
and the solid phase. The fluid temperature obeys the porous media Navier Stokes energy equation with
both advection and diffusion of the fluid energy. The inlet boundary condition is set to a
`FVNeumannBC` to specify an incoming flux. An alternative would be to use a `FVDirichletBC` to specify
an inlet temperature, but depending on the numerical scheme for the computation of advected quantities
on faces, this is not as exact.

The solid temperature is by default constant in this input file, however by activating the relevant non-linear `Variable`,
`FVKernels` and `FVBCs`, this input also allows solving for the solid phase temperature.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/heated/2d-rc-heated.i

## Example inputs : straight channel with a porosity change

This channel features a change in porosity between 1 and 0.5 with either a sharp discontinuity or
a continuous variation between the two regions, depending on the porosity initial condition (`ICs` block).

Both Rhie Chow interpolation and the momentum equation feature a pressure gradient, and the superficial momentum
advection and diffusion terms feature a porosity or inverse porosity gradient. These are ill-defined near the
discontinuity. To deal with this issue, the discontinuous case uses a flux-based formulation of the pressure gradient,
`PINSFVMomentumPressureFlux`.

To study cases with continuous porosity gradients, the `smooth_porosity` boolean parameter may be provided to the kernels,
which will enable the full treatment of pressure and porosity gradients.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/porosity_jump/2d-rc-epsjump.i

## Other inputs

One of the tests to verify the conservation of mass, momentum and energy, features both a straight and
a diverging channel. The test also features postprocessors to measure the flow of the conserved quantities on
both internal and external boundaries. The geometry can be switched using the `inactive` parameter of the `Mesh` block.

!listing modules/navier_stokes/test/tests/postprocessors/conservation_PINSFV.i

## Pronghorn

Predefined temperature and pressure dependent material properties, closure relations for
the effective viscosity, thermal conductivity, drag models and other models required
for the modeling of nuclear reactors can be found in the Pronghorn application.
Access to Pronghorn may be requested [here](https://inl.gov/ncrc) by creating a
software request.
