
# MortarFrictionalWorkWearAux

## Description

The `MortarFrictionalWorkWearAux` outputs the nodal worn-out depth from the wear law of
[!cite](reye1860zapfenreibung): the normal wear of a rubbing surface element is proportional to the
frictional work it dissipates, and hence to the product of normal pressure and sliding velocity.
This auxiliary kernel computes nodal quantities which may depend on mortar quantities, i.e. variables that are the result of numerical integration over corresponding mortar segments.
The wear velocity on the boundary is

\begin{equation*}
  \dot{w} = E_{w} \dot{d} = k_{w} p_{n} ||{\dot{\boldsymbol{\tilde{v}}}_{\tau, \text{rel}}}||,
\end{equation*}

where $\dot{d} = \mu p_{n} ||{\dot{\boldsymbol{\tilde{v}}}_{\tau, \text{rel}}}||$ is the frictional dissipation rate density for Coulomb sliding, $E_{w}$ is the energy wear coefficient, $k_{w} = \mu E_{w}$ is the local wear coefficient, $p_n$ is the normal contact pressure, $\mu$ is the coefficient of friction, and $\dot{\boldsymbol{\tilde{v}}}_{\tau, \text{rel}}$ is the tangential relative velocity computed in the mortar sense.
The pressure-velocity form is widely known as Archard's law after [!cite](archard1953contact), who
derived it from asperity contact.
Potential applications of this mortar auxiliary kernel includes the modification of contacting surface shapes due to fretting wear.

## Input example

Creation of auxiliary variables, i.e. nodal worn-out depth:

!listing test/tests/mortar_aux_kernels/block-dynamics-aux-wear.i block=AuxVariables/worn_depth

Creation of auxiliary kernel for computing worn-out depth:

!listing test/tests/mortar_aux_kernels/block-dynamics-aux-wear.i block=AuxKernels/worn_depth

!syntax parameters /AuxKernels/MortarFrictionalWorkWearAux

!syntax inputs /AuxKernels/MortarFrictionalWorkWearAux

!syntax children /AuxKernels/MortarFrictionalWorkWearAux

!bibtex bibliography
