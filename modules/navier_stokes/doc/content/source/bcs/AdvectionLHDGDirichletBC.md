# AdvectionLHDGDirichletBC

`AdvectionLHDGDirichletBC` supplies an L-HDG inlet flux using the prescribed
scalar [!param](/BCs/AdvectionLHDGDirichletBC/functor) and the prescribed
[!param](/BCs/AdvectionLHDGDirichletBC/face_velocity). The cell
[!param](/BCs/AdvectionLHDGDirichletBC/velocity) material property keeps the
interface consistent with [AdvectionLHDGKernel.md], while face upwinding uses
only `face_velocity`. Following the L-HDG Dirichlet convention, the otherwise
unused facet scalar unknown is constrained to zero.

!syntax parameters /BCs/AdvectionLHDGDirichletBC

!syntax inputs /BCs/AdvectionLHDGDirichletBC

!syntax children /BCs/AdvectionLHDGDirichletBC
