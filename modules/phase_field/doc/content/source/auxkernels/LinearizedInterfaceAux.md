# LinearizedInterfaceAux

!syntax description /AuxKernels/LinearizedInterfaceAux

## Overview

This auxkernel calculates the order parameter value from the linearized interface converted variable according to the expression

\begin{equation}
    \phi_i = \frac{1}{2} \left[ 1 + \tanh\left( \frac{\psi_i}{\sqrt{2}} \right) \right],
\end{equation}

where $\phi_i$ is the order parameter and $\psi_i$ is the transformed variable.

## Example Input File Syntax

The auxkernel is defined in the input file using the syntax:

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/grain_growth_linearized_interface.i block=AuxKernels/gr0

Alternatively, this auxkernel is added automatically by the [GrainGrowthLinearizedInterfaceAction](/GrainGrowthLinearizedInterfaceAction.md).

!syntax parameters /AuxKernels/LinearizedInterfaceAux

!syntax inputs /AuxKernels/LinearizedInterfaceAux

!syntax children /AuxKernels/LinearizedInterfaceAux
