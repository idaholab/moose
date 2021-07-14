# PCNSFVStrongBC

!syntax description /FVBCs/PCNSFVStrongBC

## Overview

This object accepts functions describing boundary values for pressure, fluid
temperature, and superficial velocity. The superficial velocity functions can
also be used to supply superficial momentum information insteady by setting
`velocity_function_includes_rho = true`. If no function is provided for a
quantity, the boundary value of that quantity will be determined by
extrapolating from the neighboring boundary cell centroid using cell centroid
value and gradient information. From this mix of user-provided functions and
extrapolated boundary values for pressure, fluid temperature, and superficial
velocity/momentum, the fluxes for mass, momentum, energy, and even passive
scalars can be computed.

!syntax parameters /FVBCs/PCNSFVStrongBC

!syntax inputs /FVBCs/PCNSFVStrongBC

!syntax children /FVBCs/PCNSFVStrongBC
