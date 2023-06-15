# LinearFrictionFactorFunctorMaterial

!syntax description /FunctorMaterials/LinearFrictionFactorFunctorMaterial

This class allows to compute the friction coefficient $W$ that is used
as:

!equation
-\epsilon \nabla p = W \rho \vec{v}_I

in the porous flow equations where all symbols have their common meaning. The
friction factor is a diagonal tensor computed by:

!equation
\vec{W} = \vec{A} f(\vec{r}, t) + \vec{B} g(\vec{r}, t) \| \vec{v}_I\|,

where $\vec{A}$ and $\vec{B}$ are constant vectors provided by the user,
$f$ and $g$ are scalar functors. 

!syntax parameters /FunctorMaterials/LinearFrictionFactorFunctorMaterial

!syntax inputs /FunctorMaterials/LinearFrictionFactorFunctorMaterial

!syntax children /FunctorMaterials/LinearFrictionFactorFunctorMaterial
