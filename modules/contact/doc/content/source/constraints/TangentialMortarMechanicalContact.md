# TangentialMortarMechanicalContact

The `TangentialMortarMechanicalContact` class is used to apply tangential stresses
to the displacement residuals. The tangential stress variable (a Lagrange
multipler) is specified using the `variable` parameter. The displacement variable
must be specified either using the `secondary_variable` or `primary_variable`
parameter. You must create as many `TangentialMortarMechanicalContact` classes as
dimensions in your simulation, e.g. for a two dimensional simulation there must
be `TangentialMortarMechanicalContact` instances for both x and y displacement components.

!syntax description /Constraints/TangentialMortarMechanicalContact

!syntax parameters /Constraints/TangentialMortarMechanicalContact

!syntax inputs /Constraints/TangentialMortarMechanicalContact

!syntax children /Constraints/TangentialMortarMechanicalContact
