# CNSFVHLLCFluidEnergyImplicitBC

!syntax description /FVBCs/CNSFVHLLCFluidEnergyImplicitBC

## Overview

This object implements an implicit boundary condition for the fluid energy
equation when using a Harten-Lax-Van Leer-Contact (HLLC) formulation. Implicit
means that the boundary condition uses no external/explicit boundary condition
information, e.g. no external input in the boundary condition block like
Dirichlet or Neumann values; instead the boundary condition forms its boundary
flux based off information from the domain interior, e.g. information from
neighboring interior cell centroids such as cell center values and
gradients. For a description of HLLC flux computation please see [CNSFVHLLCBase.md].

!syntax parameters /FVBCs/CNSFVHLLCFluidEnergyImplicitBC

!syntax inputs /FVBCs/CNSFVHLLCFluidEnergyImplicitBC

!syntax children /FVBCs/CNSFVHLLCFluidEnergyImplicitBC
