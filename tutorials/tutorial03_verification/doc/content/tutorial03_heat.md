# Heat Conduction

!---

## Theory: Heat Equation

!equation
\rho(\vec{x}) c(\vec{x})\frac{\partial T}{\partial t} = \nabla k(t,\vec{x}) \nabla T + \dot{q}\,\text{for}\,\vec{x} \in \Omega,

where $T$ is temperature, $t$ is time, $\vec{x}$ is the vector of spatial coordinates, $\rho$ is the
density, $c$ is the specific heat capacity, $k$ is the thermal conductivity, $\dot{q}$ is a heat source,
and $\Omega$ is the spatial domain.

Boundary conditions are defined on the boundary of the
domain $\partial \Omega$, the type and application of which will be discussed for the specific
application of the heat equation throughout the tutorial.

!---

## Theory: Weak Form

The weak form of the heat equation is necessary to utilize [!ac](FEM), this is done by integrating
over the domain ($\Omega$) and multiplying by a set of test functions ($\phi_i$). Performing these
steps result in the weak form as follows, provided in inner-product notation.


!equation
\lparen \phi_i, \rho(\vec{x}) c(\vec{x})\frac{\partial T_h}{\partial t} \rparen +
\lparen \nabla \phi_i, k(t,\vec{x}) \nabla T_h \rparen -
\langle \phi_i, k(t,\vec{x}) \nabla T_h \cdot \hat{n} \rangle +
\lparen \phi_i, \dot{q} \rparen = 0\,\forall{\phi_i},

where $\phi_i$ are the finite element test functions, $T_h$ is the finite element solution, and
$\hat{n}$ is the boundary outward facing normal vector. The boundary term is a result of using
integration by parts and the divergence theorem.

This derivation also includes provisions for the application of a Dirichlet boundary conditions,
the details of which are not not necessary for this tutorial.

!---

## Practice: Kernel Objects

In MOOSE, volume terms are represent by `Kernel` objects and added within the `[Kernels]` input
block. For this problem the necessary objects exist within the heat conduction module.

$\lparen \phi_i, \rho(\vec{x}) c(\vec{x})\frac{\partial T_h}{\partial t} \rparen\implies$ `HeatConductionTimeDerivative`

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Kernels remove=Kernels/T_cond Kernels/T_source

!---

## Practice: Kernel Objects

$\lparen \nabla \phi_i, k(t,\vec{x}) \nabla T_h \rparen\implies$ `HeatConduction`

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Kernels remove=Kernels/T_time Kernels/T_source

$\lparen \phi_i, \dot{q} \rparen\implies$ `HeatSouce`

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Kernels remove=Kernels/T_time Kernels/T_cond

!---

## Practice: BoundaryCondition Objects

In MOOSE, boundary terms are represented by `BoundaryCondition` objects and added within the `[BCs]`
input block. For this problem the necessary objects exist within the framework.


$-\langle \phi_i, k(t,\vec{x}) \nabla T_h \cdot \hat{n} \rangle\implies$ `NeumannBC`

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=BCs remove=BCs/bottom

!---

## Practice: BoundaryCondition Objects

The Dirichlet conditions, which prescribes a value on a boundary can also be utilized.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=BCs remove=BCs/top
