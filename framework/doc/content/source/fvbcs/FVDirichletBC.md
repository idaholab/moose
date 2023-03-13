# FVDirichletBC

!syntax description /FVBCs/FVDirichletBC

## Overview

Dirichlet boundary conditions impose the boundary condition $u=g$, where $g$ is a constant. This boundary condition is imposed weakly, through the value of the
flux.

Note that an upwinding scheme that may be used by flux kernels will affect how the Dirichlet value is applied to the interface. Upwinding schemes can result in the boundary solution being different than the specified Dirichlet value. In order to
obtain the desired boundary value, it is necessary to use a FVNeummannBC to specify
the flux.

!syntax parameters /FVBCs/FVDirichletBC

!syntax inputs /FVBCs/FVDirichletBC

!syntax children /FVBCs/FVDirichletBC
