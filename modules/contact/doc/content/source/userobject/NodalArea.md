# NodalArea

## Description

<!-- The `NodalArea` computes the nodal area along the boundaries. This object is typically utilized in contact simulations while computing the contact pressure along the interface
(see [ContactPressureAux](auxkernels/ContactPressureAux.md)). -->

The `NodalArea` object computes the tributary area of each node on a contact surface, which is used in node/face contact calculations. This object computes the value of a AuxVariable, but is implemented as a UserObject because the calculations involve iterations on the element faces, rather than the nodes. Note that the [Contact](Contact/index.md) action sets this object up automatically, so it is typically not necessary to include this in an input file.


!syntax parameters /UserObjects/NodalArea

!syntax inputs /UserObjects/NodalArea

!syntax children /UserObjects/NodalArea
