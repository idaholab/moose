# KKSPhaseConcentrationDerivatives

Kim-Kim-Suzuki (KKS) nested solve material (part 2 of 2). KKSPhaseConcentrationDerivatives computes the partial derivatives of phase concentrations w.r.t global concentrations and phase parameters, for example, $\frac{\partial c_a}{\partial c}$, where $c$ is the global concentration and $c_a$ is the phase concentration of species $c$ in phase $a$. Another example is $\frac{\partial c_b}{\eta}$ where $\eta$ is one of the two phase parameters in the model and $c_b$ is the phase concentration of species $c$ in phase $b$. These partial derivatives are used in KKS kernels as chain rule terms in the residual and Jacobian. The expressions for the derivatives were presented in [!cite](kim_phase-field_1999)

## Example input:

!listing kks_example_nested.i block=Materials

## Class Description

!syntax description /Materials/KKSPhaseConcentrationDerivatives

!syntax parameters /Materials/KKSPhaseConcentrationDerivatives

!syntax inputs /Materials/KKSPhaseConcentrationDerivatives

!syntax children /Materials/KKSPhaseConcentrationDerivatives
