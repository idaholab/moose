# Materials System Overview

Time derivative of a material property could be needed for transient calculations.
MOOSE provides a function getMaterialPropertyDot that can be called in kernels, bcs, user objects, etc,
which returns a reference of a time derivative of a real scalar material property.

!alert note
getMaterialPropertyDot currently only supports real scalar (Real) material property.

!alert note
getMaterialPropertyDot is not available in Materials.

The time derivative obtained with getMaterialPropertyDot is consistent with the time integration scheme used by the calculation.
It is noted that calling this function will make the material property stateful.
MOOSE will keep three copies of the property on all quadrature points for the current, old and older
properties that are used for evaluating the time derivative.

# Materials System

!syntax list /Materials objects=True actions=False subsystems=False

!syntax list /Materials objects=False actions=False subsystems=True

!syntax list /Materials objects=False actions=True subsystems=False
