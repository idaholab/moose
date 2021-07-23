# Heat Conduction

## Introduction

To begin this tutorial an overview of the transient heat equation is presented, as it will be
used as the basis of the tutorial throughout the various steps. This equation has already been
implemented and is provided in the [heat conduction module](modules/heat_conduction/index.md). For
a complete description please refer to the module specific documentation.

## Transient Heat Equation

!equation id=tutorial03-heat-eq
\rho(\vec{x}) c(\vec{x})\frac{\partial T}{\partial t} = \nabla k(t,\vec{x}) \nabla T + \dot{q}\,\text{for}\,\vec{x} \in \Omega

$T$ is temperature, $t$ is time, $\vec{x}$ is the vector of spatial coordinates, $\rho$ is the
density, $c$ is the specific heat capacity, $k$ is the thermal conductivity, $\dot{q}$ is a heat source,
and $\Omega$ is the spatial domain. Boundary conditions are defined on the boundary of the
domain $\partial \Omega$, the type and application of which will be discussed for the specific
application of the heat equation throughout the tutorial.

The weak form ([see Finite Elements Principles](finite_element_concepts/fem_principles.md)) of
[tutorial03-heat-eq], in inner-product notation, is given by:

!equation id=tutorial03-heat-eq-weak
\lparen \phi_i, \rho(t, \vec{x}) c(t, \vec{x})\frac{\partial T_h}{\partial t} \rparen +
\lparen \nabla \phi_i, k(t,\vec{x}) \nabla T_h \rparen -
\langle \phi_i, k(t,\vec{x}) \nabla T_h \cdot \hat{n} \rangle +
\lparen \phi_i, \dot{q} \rparen = 0\,\forall{\phi_i},

where $\phi_i$ are the finite element test functions, $T_h$ is the finite element solution, and
$\hat{n}$ is the boundary outward facing normal vector. The boundary term is a result of using
integration by parts and the divergence theorem during the weak form derivation from
[tutorial03-heat-eq].

!content pagination previous=tutorial03_verification/index.md
                    next=tutorial03_verification/step02_fem_convergence.md
