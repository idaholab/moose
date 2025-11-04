# KKSPhaseConcentrationMultiPhaseDerivatives

Kim-Kim-Suzuki (KKS) nested solve material for multiphase models (part 2 of 2). KKSPhaseConcentrationMultiPhaseDerivatives computes the partial derivatives of phase concentrations w.r.t global concentrations and phase parameters, for example, $\frac{\partial c_{i,p}}{\partial c_i}$, where $c_i$ is the global concentration and $c_{i,p}$ is the phase concentration of species $i$ in phase $p$. Another example is $\frac{\partial c_{i,p}}{\eta_p}$ where $\eta_p$ is a phase parameter in the model. These partial derivatives are used in KKS kernels as chain rule terms in the residual and Jacobian. The expressions for the derivatives were presented in [!cite](kim_phase-field_1999).

## Example input:

!listing kks_example_multiphase_nested.i block=Materials

## Class Description

!syntax description /Materials/KKSPhaseConcentrationMultiPhaseDerivatives

!syntax parameters /Materials/KKSPhaseConcentrationMultiPhaseDerivatives

!syntax inputs /Materials/KKSPhaseConcentrationMultiPhaseDerivatives

!syntax children /Materials/KKSPhaseConcentrationMultiPhaseDerivatives
