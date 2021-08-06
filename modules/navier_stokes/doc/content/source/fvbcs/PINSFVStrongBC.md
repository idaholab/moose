# PINSFVStrongBC

!syntax description /FVBCs/PINSFVStrongBC

## Overview

This object is very similar to [PCNSFVStrongBC.md]. However, this object is
designed for use in tandem with an incompressible/weakly-compressible set of
objects (see [PINSFVMomentumAdvection.md]) while [PCNSFVStrongBC.md] is meant
for use in tandem with a fully compressible set of objects (see [PCNSFVKT.md]).

This object accepts functions describing boundary values for pressure and
superficial velocity. If no function is provided for a quantity, the boundary
value of that quantity will be determined by extrapolating from the neighboring
boundary cell centroid using cell centroid value and gradient information. From
this mix of user-provided functions and extrapolated boundary values for
pressure and superficial velocity, the fluxes for mass and momentum can be
computed.

!syntax parameters /FVBCs/PINSFVStrongBC

!syntax inputs /FVBCs/PINSFVStrongBC

!syntax children /FVBCs/PINSFVStrongBC
