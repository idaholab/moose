# NormalMortarMechanicalContact

The `NormalMortarMechanicalContact` class is used to apply normal contact forces
to the displacement residuals. The contact pressure variable (a Lagrange
multipler) is specified using the `variable` parameter. The displacement variable
must be specified either using the `secondary_variable` or `primary_variable`
parameter. You must create as many `NormalMortarMechanicalContact` classes as
dimensions in your simulation, e.g. for a two dimensional simulation there must
be `NormalMortarMechanicalContact` instances for both x and y displacement components.

!syntax description /Constraints/NormalMortarMechanicalContact

!syntax parameters /Constraints/NormalMortarMechanicalContact

!syntax inputs /Constraints/NormalMortarMechanicalContact

!syntax children /Constraints/NormalMortarMechanicalContact
