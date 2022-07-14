# The Navier-Stokes Module

!style halign=center fontsize=120%
!datetime today format=%B %Y

!---

# Overview

Main capabilities:

- Incompressible fluid flows
- Weakly-compressible fluid flows
- Compressible fluid flows
- Free flows and flows in porous media describing homogenized structures

Utilizies the following techniques:

- Continuous Finite Element Discretization (maintained, but not intensely devleoped)
- Finite Volume Discretization (current development direction)

!---

# The Navier-Stokes equations
  style=font-size:26pt

!style! fontsize=80%

- Conservation of +mass+:

!equation id=mass-eq
\frac{\partial \rho}{\partial t} + \nabla \cdot \left( \rho \vec{u} \right)  = 0

- Conservation of (linear) +momentum+:

!equation id=momentum-eq
\frac{\partial \rho  \vec{u}}{\partial t} + \nabla \cdot \left(\gamma^{-1}  \rho \vec{u} \otimes \vec{u}\right) = -\gamma\nabla p + \gamma  \rho \vec{g} -  W  \vec{u}_I

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
- $p$ - pressure
- $W$ - friction tensor (using correlations)
!style-end!

!col-end!
!row-end!

!style! fontsize=80%
- If $\gamma=1$ then $\vec{u}=\vec{u}_I$ and we get the original Navier-Stokes equations back.
!style-end!


!---

# The Navier-Stokes equations (contd.)
  style=font-size:26pt

- These are supplemented by the conservarion of +energy+:

  +Assumptions:+ Weakly-compressible fluid, kinteic energy of the fluid can be neglected

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

# The Navier-Stokes equations (contd.)
  style=font-size:26pt

The equations are extended with initial and boundary conditions:

- Few examples:

  - Inlet: $\vec{u}=\vec{u}_{inlet}, \quad T=T_{inlet}, \quad \vec{r} \in \partial\Omega_{inlet}$
  - Outlet: $p = p_{outlet}, \quad \vec{r} \in \partial\Omega_{outlet}$
  - No-slip, insulated walls: $\vec{u}=\vec{0}, \quad \kappa_f \nabla T \cdot \vec{n} = 0,  \quad \vec{r} \in \partial\Omega_{wall}$
  - Heat flux on the walls: $\quad -\kappa_f \nabla T \big|_{\partial \Omega} \cdot \vec{n} = 0,  \quad \vec{r} \in \partial\Omega_{heated}$
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

!col-end!

!row-end!

!---

# Building Input Files

The building blocks in MOOSE for terms in the PDEs are +Kernels+ for FE or +FVKernels+ for FV:

!style! fontsize=80%
!equation
\underbrace{\frac{\partial \rho  \vec{u}}{\partial t}}_{\text{PINSFMomentumTimeDerivative}}
+ \underbrace{\nabla \cdot \left(\gamma^{-1}  \rho \vec{u} \otimes \vec{u}\right)}_{\text{PINSFVMomentumAdvection}} =
\underbrace{-\gamma\nabla p}_{\text{PINSFVMomentumPressure}}
+ \underbrace{\gamma\rho \vec{g}}_{\text{PINSFVMomentumGravity~}}
\underbrace{-  W  \vec{u}_I}_{\text{PINSFVMomentumFriction}}

The building blocks in MOOSE for boundary conditions are +BCs+ for FE or +FVBCs+ for FV:

- Inlet Velocity, Temperature: $\text{FVFunctionDirichletBC}$
- Outlet pressure: $\text{Something}$
- Heatflux: $\text{FVFunctionNeumannBC}$
- Freeslip: $\text{MomentumFreeSlipBC}$
- ...


!style-end!


