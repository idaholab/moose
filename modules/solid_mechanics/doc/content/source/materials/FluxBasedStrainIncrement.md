# FluxBasedStrainIncrement

!syntax description /Materials/FluxBasedStrainIncrement

The strain is computed from flux instead of the displacements. The gradient of the flux is
first computed from the gradient of each component of the flux specified by the user:

!equation
\nabla F = (\nabla F_x, \nabla F_y, \nabla F_z)

where:

- $F_x$ is the x-component of the flux specified with the [!param](/Materials/FluxBasedStrainIncrement/xflux) parameter
- $F_y$ is the y-component of the flux, optionally specified with the [!param](/Materials/FluxBasedStrainIncrement/yflux) parameter
- $F_z$ is the z-component of the flux, optionally specified with the [!param](/Materials/FluxBasedStrainIncrement/zflux) parameter

The strain increment tensor is then computed as:

!equation
\boldsymbol{\epsilon}_{inc} = -\dfrac{1}{2}(\nabla F + ^T(\nabla F)) *  (1.0 - G) * \text{dt};

where $G$ is the grain boundary order parameter.

!alert note
The computation of the flux based strain increment assumes a first order Euler time integration scheme.

!syntax parameters /Materials/FluxBasedStrainIncrement

!syntax inputs /Materials/FluxBasedStrainIncrement

!syntax children /Materials/FluxBasedStrainIncrement
