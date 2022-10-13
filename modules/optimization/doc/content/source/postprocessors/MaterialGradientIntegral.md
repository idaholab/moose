# MaterialGradientIntegral

!syntax description /Postprocessors/MaterialGradientIntegral

## Overview

The gradient of the diffusion kernel with respect to a material parameter is given by the equation
\begin{equation}\label{eq:grad}
\lambda\frac{\partial \hat{\text{R}}}{\partial\mathbf{p}} =  \int\nabla\lambda^{\top} \cdot \frac{\partial \kappa}{{\partial\mathbf{p}}}\cdot\nabla T~\text{d}\Omega,
\end{equation}

This equation is derived here [!eqref](theory/InvOptTheory.md#eq:kappaLambda) on the optimization theory page.



## Example Input File Syntax

An examples of how the `MaterialGradientIntegral` is used is given here:

!listing test/tests/optimizationreporter/material/adjoint.i block=Postprocessors/pp_adjoint_grad id=input

[!param](/Postprocessors/MaterialGradientIntegral/adjoint_var) is the variable `adjoint_var` being solved in this input file.  [!param](/Postprocessors/MaterialGradientIntegral/forward_var) is the AuxVariable `forward_var` containing the solution from the forward solve.  [!param](/Postprocessors/MaterialGradientIntegral/material_derivative) given by the material property `thermal_conductivity_deriv`.  In this problem the thermal conductivity is constant and so `thermal_conductivity_deriv` is equal to one over the entire domain.  

!syntax parameters /Postprocessors/MaterialGradientIntegral

!syntax inputs /Postprocessors/MaterialGradientIntegral

!syntax children /Postprocessors/MaterialGradientIntegral
