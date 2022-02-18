
# MortarArchardsLawAux

## Description

The `MortarArchardsLawAux` outputs the nodal worn-out depth based on a well-known wear equation by Archard. This auxiliary kernel computes nodal quantities which may depend on mortar quantities, i.e. variables that are the result of numerical integration over corresponding mortar segments.
The wear velocity on the boundary may be defined as:

\begin{equation*}
  \dot{w} = \frac{k_{w}}{\mu} \dot{d} = E_{w} \dot{d},
\end{equation*}

where $\dot{d} = \mu p_{n} ||{\dot{\boldsymbol{\tilde{v}}}_{\tau, \text{rel}}}||$ is the frictional dissipation rate density, $E_{w}$ is the energy wear coefficient, $k_{w}$ is the local wear coefficient, $p_n$ is the normal contact pressure, $\mu$ is the coefficient of friction, and $\dot{\boldsymbol{\tilde{v}}}_{\tau, \text{rel}}$ is the tangential relative velocity computed in the mortar sense.
Potential applications of this mortar auxiliary kernel includes the modification of contacting surface shapes due to fretting wear.

## Input example

Creation of auxiliary variables, i.e. nodal worn-out depth:

!listing test/tests/mortar_aux_kernels/block-dynamics-aux-wear.i block=AuxVariables/worn_depth

Creation of auxiliary kernel for computing worn-out depth:

!listing test/tests/mortar_aux_kernels/block-dynamics-aux-wear.i block=AuxKernels/worn_depth

!syntax parameters /AuxKernels/MortarArchardsLawAux

!syntax inputs /AuxKernels/MortarArchardsLawAux

!syntax children /AuxKernels/MortarArchardsLawAux
