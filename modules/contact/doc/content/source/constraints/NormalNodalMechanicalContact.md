# NormalNodalMechanicalContact

Similar to the
[`NormalMortarMechanicalContact`](/NormalMortarMechanicalContact.md) object
except the contact forces are applied through a node-face geometric
discretization as opposed to a mortar discretization. Some
[numerical experiments](modules/contact/index.md#frictionless_table) show that
non-linear solve convergence with the node-face discretization for contact forces
is comparable or only slightly worse than the mortar application.

!syntax description /Constraints/NormalNodalMechanicalContact

!syntax parameters /Constraints/NormalNodalMechanicalContact

!syntax inputs /Constraints/NormalNodalMechanicalContact

!syntax children /Constraints/NormalNodalMechanicalContact

!bibtex bibliography
