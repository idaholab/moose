# TangentialNodalLMMechanicalContact

Similar to the
[`TangentialMortarLMMechanicalContact`](/TangentialMortarLMMechanicalContact.md) object
except the constraints are enforced pointwise at nodes instead of in an integral
way using the mortar method. Some
[numerical experiments](modules/contact/index.md#frictional_table) suggest
that the pointwise method of enforcing tangential contact constraints displays worse
non-linear convergence than the mortar method.

!syntax description /Constraints/TangentialNodalLMMechanicalContact

!syntax parameters /Constraints/TangentialNodalLMMechanicalContact

!syntax inputs /Constraints/TangentialNodalLMMechanicalContact

!syntax children /Constraints/TangentialNodalLMMechanicalContact

!bibtex bibliography
