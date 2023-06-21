# WallFrictionFunctionMaterial

!syntax description /Materials/WallFrictionFunctionMaterial

The Darcy friction factor $f_D$ is directly computed from the spatial and temporal value of the user-input function $f$:

!equation
f_D = f(x,y,z,t)

with $x,y,z$ the spatial coordinates and $t$ the current simulation time.
As the function is not supposed to (and should not) depend on any variable,
this material also defines +zero+ material properties for the derivatives of the friction factor with regards to:

- $\alpha \rho A$
- $\alpha \rho u A$
- $\alpha \rho E A$
- $\beta$ if the [!param](/Materials/WallFrictionFunctionMaterial/beta) parameter is set by the user

!syntax parameters /Materials/WallFrictionFunctionMaterial

!syntax inputs /Materials/WallFrictionFunctionMaterial

!syntax children /Materials/WallFrictionFunctionMaterial
