# AdvectionIPHDGDirichletBC

This class adds an advective flux proportional to the Dirichlet value supplied by [!param](/BCs/AdvectionIPHDGDirichletBC/functor). This Dirichlet value could correspond to the concentration of a passive scalar. The advective flux can be scaled by [!param](/BCs/AdvectionIPHDGDirichletBC/coeff), which might represent something like a density.

!alert warning
Note that we do not check for outflow conditions in this object; if the Dirichlet boundary condition is imposed on what turns out to be an outflow boundary, the Dirichlet value will be imposed which may not be what the user wants.

!syntax parameters /BCs/AdvectionIPHDGDirichletBC

!syntax inputs /BCs/AdvectionIPHDGDirichletBC

!syntax children /BCs/AdvectionIPHDGDirichletBC
