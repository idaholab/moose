# ComputeConcentrationDependentElasticityTensor

!syntax description /Materials/ComputeConcentrationDependentElasticityTensor

The concentration variable $c$, specified using the [!param](/Materials/ComputeConcentrationDependentElasticityTensor/c),
acts as a mixing function for the two elasticity tensors, $\boldsymbol{C_0}$ and $\boldsymbol{C_1}$, specified in the [!param](/Materials/ComputeConcentrationDependentElasticityTensor/C0_ijkl) and [!param](/Materials/ComputeConcentrationDependentElasticityTensor/C1_ijkl)
vector parameters. See [ComputeElasticityTensor.md] for an explanation about the fill methods, e.g. how to input a tensor as a vector parameter.

The elasticity tensor is then:

!equation
\boldsymbol{C} = \boldsymbol{C_0} + c (\boldsymbol{C_1} - \boldsymbol{C_0})

the derivative tensor of the elasticity tensor with regards to the concentration variable is then naturally:

!equation
\dfrac{\partial \boldsymbol{C}}{\partial c} = \boldsymbol{C_1} - \boldsymbol{C_0}

!syntax parameters /Materials/ComputeConcentrationDependentElasticityTensor

!syntax inputs /Materials/ComputeConcentrationDependentElasticityTensor

!syntax children /Materials/ComputeConcentrationDependentElasticityTensor
