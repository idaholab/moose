#HyperbolicViscoplasticityStressUpdate
!syntax description /Materials/HyperbolicViscoplasticityStressUpdate


## Description
!include docs/content/documentation/modules/tensor_mechanics/common/supplementalRadialReturnStressUpdate.md

This uniaxial viscoplasticity class computes the plastic strain as a stateful material property.  The constitutive equation for scalar plastic strain rate used in this model is
$$
\dot{p} = \phi (\sigma_e , r) = \alpha sinh \beta (\sigma_e -r - \sigma_y)
$$

This class is based on the implicit integration algorithm in \cite{dunne2005introduction} pg. 162 - 163.

!syntax parameters /Materials/HyperbolicViscoplasticityStressUpdate

!syntax inputs /Materials/HyperbolicViscoplasticityStressUpdate

!syntax children /Materials/HyperbolicViscoplasticityStressUpdate

## References
\bibliographystyle{unsrt}
\bibliography{docs/bib/tensor_mechanics.bib}
