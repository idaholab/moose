# ACGrGrPolyLinearizedInterface

!syntax description /Kernels/ACGrGrPolyLinearizedInterface

## Overview

This kernel implements the bulk portion of the residual for the Allen-Cahn equation from the multiphase grain growth model using the linearized interface variable substitution. It is the linearized interface version of the [ACGrGrPoly](/ACGrGrPoly.md) kernel. The grain growth model comes from [!cite](moelans_quantitative_2008) and the linearized interface from [!cite](glasner2001nonlinear).

The kernel inherits the same residual function from [ACGrGrPoly](/ACGrGrPoly.md) but uses a linearized interface substitution. The substitution is defined in the material property `this_op`. The most common one is
\begin{equation}
    \phi_i = \frac{1}{2} \left[ 1 + \tanh\left( \frac{\psi_i}{\sqrt{2}} \right) \right],
\end{equation}
where $\phi_i$ is the order parameter and $\psi_i$ is the transformed variable. The substitution is implemented in the material [LinearizedInterfaceFunction](/LinearizedInterfaceFunction.md).


## Example Input File Syntax

The kernel can be used according to

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/grain_growth_linearized_interface.i block=Kernels/phi0_ACInt

However, it can also be created automatically using the [GrainGrowthLinearizedInterfaceAction](/GrainGrowthLinearizedInterfaceAction.md).

!syntax parameters /Kernels/ACGrGrPolyLinearizedInterface

!syntax inputs /Kernels/ACGrGrPolyLinearizedInterface

!syntax children /Kernels/ACGrGrPolyLinearizedInterface

!bibtex bibliography
