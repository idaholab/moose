# ACInterfaceChangedVariable

!syntax description /Kernels/ACInterfaceChangedVariable

## Overview

This kernel implements the gradient energy contribution of the Allen-Cahn equation residual for the isotropic mobility case, using a change of variable. It is the change of variable version of the [ACInterface](/ACInterface.md) kernel. It is used with linearized interface grain growth model from [!cite](glasner2001nonlinear).

With a linearized interface, the weak form is

\begin{equation}
\left( \kappa_i \nabla \phi_i, \nabla (L_i \zeta_m ) \right) = \left( \kappa_i \nabla \psi_i \frac{\partial \phi_i}{\partial \psi_i}, \nabla (L_i \zeta_m ) \right),
\end{equation}

where $\kappa_i$ (`kappa_name`) is the gradient energy coefficent, $\phi_i$ the
non-conserved non-linear order parameter variable, $\psi_i$ is the transformed variable the kernel is acting on, $L_i$
(`mob_name`) is the scalar (isotropic) mobility associated with the order
parameter, and $\zeta_m$ is the test function. The derivative of the change of variable function $\frac{\partial \phi_i}{\partial \psi_i}$ is defined using a derivative parsed material such as [LinearizedInterfaceFunction](/LinearizedInterfaceFunction.md).

## Example Input File Syntax

The kernel is used in an input file with the syntax

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/grain_growth_linearized_interface.i block=Kernels/phi0_dot

Alternatively, this kernel is added automatically by the [GrainGrowthLinearizedInterfaceAction](/GrainGrowthLinearizedInterfaceAction.md).

!syntax parameters /Kernels/ACInterfaceChangedVariable

!syntax inputs /Kernels/ACInterfaceChangedVariable

!syntax children /Kernels/ACInterfaceChangedVariable
