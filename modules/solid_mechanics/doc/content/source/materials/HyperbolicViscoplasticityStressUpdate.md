# Hyperbolic Viscoplasticity Stress Update

!syntax description /Materials/HyperbolicViscoplasticityStressUpdate

## Description

!include modules/tensor_mechanics/common/supplementalRadialReturnStressUpdate.md

This uniaxial viscoplasticity class computes the plastic strain as a stateful material property.  The
constitutive equation for scalar plastic strain rate used in this model is
\begin{equation}
\dot{p} = \phi (\sigma_e , r) = \alpha sinh \beta (\sigma_e -r - \sigma_y)
\end{equation}

This class is based on the implicit integration algorithm in [!cite](dunne2005introduction)
pg. 162--163.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/recompute_radial_return/uniaxial_viscoplasticity_incrementalstrain.i
         block=Materials/viscoplasticity

`HyperbolicViscoplasticityStressUpdate` must be run in conjunction with the inelastic strain return
mapping stress calculator as shown below:

!listing modules/tensor_mechanics/test/tests/recompute_radial_return/uniaxial_viscoplasticity_incrementalstrain.i
         block=Materials/radial_return_stress

!syntax parameters /Materials/HyperbolicViscoplasticityStressUpdate

!syntax inputs /Materials/HyperbolicViscoplasticityStressUpdate

!syntax children /Materials/HyperbolicViscoplasticityStressUpdate

!bibtex bibliography
