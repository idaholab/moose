# The MOOSE Navier-Stokes Module

!style halign=center fontsize=120%
!datetime today format=%B %Y

!---

# Overview
  style=font-size:28pt

!style! fontsize=80%

!row!
!col! width=60%

Main capabilities:

- Fluid types:

  - Incompressible
  - Weakly-compressible
  - Compressible
  - Two-phase mixture model

- Flow regimes:

  - Laminar
  - Turbulent

- Flow types:

  - Free flows
  - Flows in porous media describing homogenized structures

!col-end!

!col! width=40%

Utilizes the following techniques:

- Stabilized Continuous Finite Element Discretization (maintained, but not intensely developed) ([!cite](peterson2018overview))
- Finite Volume Discretization with nonlinear solver ([!cite](lindsay2023moose))
- Finite Volume Discretization with linear solver

!col-end!
!row-end!

!style-end!

!---

# Applicability of each solver

Nonlinear (Newton method) finite volume

- 2D problems
- only solver with support for porous medium
- turbulence models (mixing-length, k-$\epsilon$)
- fully compressible flow problems

Linear (SIMPLE/PIMPLE) finite volume

- 3D problems

You can check that the combination of models you want is supported [in Table 2 on this page](modules/navier_stokes/index.md)
The nonlinear (SIMPLE/PIMPLE) finite volume is being replaced by the linear implementation. It is currently still only used for
two-phase flow studies.

!---

# The Porous Navier-Stokes equations  ([!cite](vafai2015handbook), [!cite](radman2021development))
  style=font-size:24pt

!style! fontsize=80%

- Conservation of +mass+:

!equation id=mass-eq
\frac{\partial \gamma \rho}{\partial t} + \nabla \cdot \left( \rho \vec{u} \right)  = 0

- Conservation of (linear) +momentum+:

!equation id=momentum-eq
\frac{\partial \rho  \vec{u}}{\partial t} + \nabla \cdot \left(\gamma^{-1}  \rho \vec{u} \otimes \vec{u}\right)
= \nabla \cdot \left(\gamma \mu_\text{eff} \left(\nabla\vec{u}_I + \left(\nabla \vec{u}_I\right)^T-\frac{2}{3}\nabla\cdot\vec{u}_I\mathbb{I}\right)\right) -\gamma\nabla p + \gamma  \rho \vec{g} -  W  \vec{u}_I

!style-end!

!row!
!col! width=30%

!style! fontsize=70%
- $\gamma$ - porosity
- $\vec{u}_I$ - real (or interstitial) velocity
- $\vec{u} \equiv \gamma \vec{u}_I$ - superficial velocity
!style-end!

!col-end!

!col! width=60%

!style! fontsize=70%
- $\rho = \rho(p,T)$ - density
- $\mu_{eff}$ - effective dynamic viscosity (laminar + turbulent)
- $p$ - pressure
- $W$ - friction tensor (using correlations)
!style-end!

!col-end!
!row-end!

!style! fontsize=80%
- If $\gamma=1$ then $\vec{u}=\vec{u}_I$ and we get the original Navier-Stokes equations back.
!style-end!


!---

# The Porous Navier-Stokes equations (contd.)
  style=font-size:26pt

- These are supplemented by the conservation of +energy+:

  +Assumptions:+ Weakly-compressible fluid, kinetic energy of the fluid can be neglected

!equation id=enthalpy-eq
\frac{\partial \gamma \rho c_p T}{\partial t} + \nabla \cdot \left(  \rho c_p T \vec{u}\right)
= \nabla \cdot \left(\kappa_f \nabla T \right) - \alpha (T - T_s) + S_{ext}

!row!
!col! width=30%

!style! fontsize=80%
- $c_p$ - specific heat
- $T$ - fluid temperature
- $\kappa_f$ - effective thermal conductivity
!style-end!

!col-end!

!col! width=60%

!style! fontsize=80%
- $\alpha$ - volumetric heat-transfer coefficient (using correlations)
- $T_s$ - solid temperature
- $S_{ext}$ - volumetric source term
!style-end!

!col-end!

!row-end!

!---

# The Porous Navier-Stokes equations (contd.)
  style=font-size:26pt

The equations are extended with initial and boundary conditions:

- Few examples:

  - Velocity Inlet: $\vec{u}=\vec{u}_{inlet}, \quad T=T_{inlet}, \quad \vec{r} \in \partial\Omega_{inlet}$
  - Pressure Outlet: $p = p_{outlet}, \quad \vec{r} \in \partial\Omega_{outlet}$
  - No-slip, insulated walls: $\vec{u}=\vec{0}, \quad \kappa_f \nabla T \cdot \vec{n} = 0,  \quad \vec{r} \in \partial\Omega_{wall}$
  - Heat flux on the walls: $\quad -\kappa_f \nabla T \big|_{\partial \Omega} \cdot \vec{n} = q^{\prime \prime},  \quad \vec{r} \in \partial\Omega_{heated}$
  - Free slip on walls: $\vec{u} \cdot \vec{n} = 0, \quad \vec{r} \in \partial\Omega_{slip}$
  - ...

!---

# Useful Acronyms

Good to know before navigating source code and documentation:

!row!
!col! width=50%

- +NS+ - +N+avier-+S+tokes
- +FV+ - +F+inite +V+olume
- +I+ - +I+ncompressible
- +WC+ - +W+eakly-+C+ompressible
- +C+ - +C+ompressible
- +P+ - +P+orous-Medium

!col-end!

!col! width=50%

Few Examples:

- +PINSFV:+ +P+orous-Medium +I+ncompressible +N+avier-+S+tokes +F+inite +V+olume
- +WCNSFV:+ +W+eakly-+C+ompressible +N+avier-+S+tokes +F+inite +V+olume
- +WCNSLinearFV:+ +W+eakly-+C+ompressible +N+avier-+S+tokes Linear +F+inite +V+olume

!col-end!

!row-end!

!---

# Building Input Files
style=font-size:28pt

The building blocks in MOOSE for terms in the PDEs are +Kernels+ for FE, +FVKernels+ for FV, and +LinearFVKernels+ for SIMPLE-FV:

!style! fontsize=80%
!equation
\underbrace{\frac{\partial \rho  \vec{u}}{\partial t}}_{\text{PINSFVMomentumTimeDerivative}}
+ \underbrace{\nabla \cdot \left(\gamma^{-1}  \rho \vec{u} \otimes \vec{u}\right)}_{\text{PINSFVMomentumAdvection}} =

!equation
\underbrace{\nabla \cdot \left(\gamma \mu_\text{eff} \left(\nabla\vec{u}_I + \left(\nabla \vec{u}_I\right)^T-\frac{2}{3}\nabla\cdot\vec{u}_I\mathbb{I}\right)\right)}_{\text{PINSFVMomentumDiffusion}}
\underbrace{-\gamma\nabla p}_{\text{PINSFVMomentumPressure}}
+ \underbrace{\gamma\rho \vec{g}}_{\text{PINSFVMomentumGravity~}}
\underbrace{-  W  \vec{u}_I}_{\text{PINSFVMomentumFriction}}

The building blocks in MOOSE for boundary conditions are +BCs+ for FE, +FVBCs+ for Newton-FV, and +LinearFVBCs+ for SIMPLE-FV:

- Inlet Velocity: $\text{INSFVInletVelocityBC}$
- Inlet Temperature: $\text{FVFunctionDirichletBC}$
- Outlet pressure: $\text{INSFVOutletPressureBC}$
- Heatflux: $\text{FVFunctionNeumannBC}$
- Freeslip: $\text{INSFVNaturalFreeSlipBC}$
- ...

!style-end!

!---

# A Simple Example: Laminar Free Flow in a Channel
style=font-size:26pt

!media navier_stokes/nsfv-channel-example.png
       style=width:100%;background:white;
       alt=A diagram showing the size, shape, and conditions of a channel.

Let us consider the following (fictional) material properties:

- $\mu=1.1$ $\text{Pa}\cdot\text{s}$
- $\rho=1.1$ $\frac{kg}{m^3}$ (we are using incompressible formulation in this case)

!---

# Detailed Input File (nonlinear/Newton FV)
style=font-size:26pt

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/2d-rc-no-slip.i

!---

# Detailed Input File (linear/SIMPLE FV)
style=font-size:26pt

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d/2d-velocity-pressure.i

!---

# Rhie-Chow Interpolation
style=font-size:26pt

- When using linear interpolation for the advecting velocity and the pressure gradient, we
  encounter a checker-boarding effect for the pressure field. Neighboring cell center solution
  values are decoupled, which is undesirable.

- +Solution for this issue:+ Rhie-Chow interpolation for
  the advecting velocity. For more information on the issue and the derivation of the interpolation
  method, see [!cite](moukalled2016finite).



!style! halign=center

!row!

!col! width=30%
!media navier_stokes/rc_bad.png
       style=width:100%;background:white;
       alt=Checkerboard pattern in the pressure field when using linear interpolation of advecting velocity and pressure gradient.
!col-end!

!col! width=40%
$\Large\xrightarrow[\mathrm{Rhie-Chow~Interpolation}]{\vec{u}_{RC,f} = \vec{u}_{AVG,f} - \left(\frac{1}{a}\right)_f\left(\nabla p_f - \overline{\nabla p}_f\right)}$
!col-end!

!col! width=30%
!media navier_stokes/rc_good.png
       style=width:100%;background:white;
       alt=The pressure field obtained using Rhie-Chow interpolation for the advecting velocity.
!col-end!

!row-end!

!style-end!

!---

# The Navier-Stokes Physics
style=font-size:26pt

A simplified syntax has been developed, it relies on the `Physics` system in MOOSE.
Each [Physics](syntax/Physics/index.md) is in charge of creating one equation. The input below shows the
[flow](WCNSFVFlowPhysics.md) and [energy conservation](WCNSFVFluidHeatTransferPhysics.md) equations.
Only the finite volume discretization, for the Newton and PIMPLE solvers, support this syntax.

!listing tests/finite_volume/pins/channel-flow/heated/2d-rc-heated-physics.i

!---

# Recommendations for Building Input Files
style=font-size:28pt

!style! fontsize=70%

- Check that the combination of models you want is supported [in Table 2 there](modules/navier_stokes/index.md)
- Start from an example. If no example, find a test that looks similar.
- If possible, use the Navier Stokes finite volume `Physics` syntax
- Use Rhie-Chow interpolation for the advecting velocity

  - Other interpolation techniques may lead to checkerboarding/instability


- Start with first-order advected-interpolation schemes (e.g. upwind)
- Make sure that the pressure is pinned for incompressible/weakly-compressible simulations in closed systems
- For monolithic solvers (the default at the moment) use a variant of LU preconditioner
- For complex monolithic systems monitor the residuals of every variable
- Try to keep the number of elements relatively low (limited scaling with LU preconditioner$^*$).
  Larger problems can be solved in a reasonable wall-time as segregated-solve techniques are introduced
- Try to utilize `porosity_smoothing_layers` or a higher value for the `consistent_scaling` if you encounter oscillatory behavior in case of simulations using porous medium

!style-end!

!style! fontsize=60%
$^*$ Because the complexity of the LU preconditioner is $N^3$ in the worst case scenario, its performance in general largely depends on the matrix sparsity pattern, and it is much more expensive compared with an iterative approach.
!style-end!

!---

# Validation studies and additional examples

[Turbulence modeling for Molten Salt Reactors](https://www.sciencedirect.com/science/article/pii/S0029549324008872)

Open-Pronghorn flow test bed (to be released soon)

!---
