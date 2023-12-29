# ComputeLagrangianLinearElasticStress

!syntax description /Materials/ComputeLagrangianLinearElasticStress

## Overview

This class implements the simple linear elastic model
\begin{equation}
      s_{ij} = C_{ijkl} \varepsilon_{kl}
\end{equation}
which returns the small stress as a linear function of the small strain.  
$C_{ijkl}$ is an elasticity tensor calculated by a [`ComputeElasticityTensor`](ComputeElasticityTensor.md) object,
with a name provided in the stress calculator input.

This model inherits from [`ComputeLagrangianObjectiveStress`](ComputeLagrangianObjectiveStress.md) and so
it will integrate an objective stress rate to provide a "hypoelastic" large deformation constitutive
response based solely on the small strain model.

## Example Input File Syntax

The key parameters are `elasticity_tensor` giving the name of the elasticity tensor and
`large_kinematics` which controls whether the model uses the simple small stress, linear elastic
update or integrates the model through an objective stress rate.  The `objective_rate`
parameters controls which objective rate is used.

The following example sets up a hypoelastic, large deformation model based on the
`truesdell` rate using the default elastic tensor name (`"elasticity_tensor"`).

!listing modules/tensor_mechanics/test/tests/lagrangian/cartesian/updated/patch/large_patch.i
         block=Materials

!syntax parameters /Materials/ComputeLagrangianLinearElasticStress

!syntax inputs /Materials/ComputeLagrangianLinearElasticStress

!syntax children /Materials/ComputeLagrangianLinearElasticStress
