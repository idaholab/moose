# NormalMortarLMMechanicalContact

Similar to the
[`NormalNodalLMMechanicalContact`](/NormalNodalLMMechanicalContact.md) object
except the constraints are enforced in an integral way via the mortar method as
opposed to pointwise at nodes. Some
[numerical experiments](modules/contact/index.md#frictionless_table) suggest
that the integral method of enforcing normal contact constraints displays worse
non-linear convergence than the pointwise method.

!syntax description /Constraints/NormalMortarLMMechanicalContact

!syntax parameters /Constraints/NormalMortarLMMechanicalContact

!syntax inputs /Constraints/NormalMortarLMMechanicalContact

!syntax children /Constraints/NormalMortarLMMechanicalContact

!bibtex bibliography
