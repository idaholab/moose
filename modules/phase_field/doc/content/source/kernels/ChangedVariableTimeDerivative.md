# ChangedVariableTimeDerivative

!syntax description /Kernels/ChangedVariableTimeDerivative

## Overview

This kernel implements the time derivative of a variable that has been converted using a change of variable transformation. So, we transform variable $\phi_i$ to $\psi_i$. It is used in kinetic equations that use a change of variable.

The time derivative becomes
\begin{equation}
    \frac{\partial \phi_i}{\partial t} = \frac{\partial \psi}{\partial t} \frac{\partial \phi}{\partial \psi}.
\end{equation}
The derivative of the transformation $\frac{\partial \phi}{\partial \psi}$ is accessed from a material property, like the linearized interface function implemented in [LinearizedInterfaceFunction](LinearizedInterfaceFunction.md)

## Example Input File Syntax

The kernel is defined in the input file using the syntax:

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/grain_growth_linearized_interface.i block=Kernels/phi0_ACInt

Alternatively, this kernel is added automatically by the [GrainGrowthLiniearizedInterfaceAction](/GrainGrowthLinearizedInterfaceAction.md).

!! Describe and include an example of how to use the ChangedVariableTimeDerivative object.

!syntax parameters /Kernels/ChangedVariableTimeDerivative

!syntax inputs /Kernels/ChangedVariableTimeDerivative

!syntax children /Kernels/ChangedVariableTimeDerivative
