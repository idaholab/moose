# ComputeStVenantKirchhoffStress

!syntax description /Materials/ComputeStVenantKirchhoffStress

## Overview

This class provides a hyperelastic St. Venant-Kirchhoff stress update
defining the 2nd Piola-Kirchhoff stress as
\begin{equation}
      S_{IJ} = C_{IJKL} E_{KL}
\end{equation}
where $E_{KL}$ is the Green-Lagrange strain and $C_{IJKL}$ is 
an elasticity tensor calculated by a [`ComputeElasticityTensor`](ComputeElasticityTensor.md) object,
with a name provided in the stress calculator input.

The model requires an isotropic elasticity tensor, as this is the only
elasticity tensor that will lead to a truly hyperelastic model.  

For small strains, when `large_kinematics = false` the model instead 
returns the standard linear elastic response, based on the 
elasticity tensor.

[example] shows a typical material response under uniaxial stress.  The 
plot shows the results in terms of the Cauchy stress and logarithmic 
strain to illustrate that the large deformation version of the
model is in fact nonlinear.

!media tensor_mechanics/stvenant.png
       id=example
       style=width:50%;float:center;padding-top:1.5%;
       caption=Response of the St. Venant-Kirchhoff model to uniaxial deformation, plotted as log strain versus Cauchy stress.

## Example Input File Syntax

The follow example configures a large deformation St. Venant-Kirchhoff
model using the default elasticity tensor name to define the
response.

!listing modules/tensor_mechanics/test/tests/lagrangian/materials/correctness/stvenantkirchhoff.i
         block=Materials

!syntax parameters /Materials/ComputeStVenantKirchhoffStress

!syntax inputs /Materials/ComputeStVenantKirchhoffStress

!syntax children /Materials/ComputeStVenantKirchhoffStress
