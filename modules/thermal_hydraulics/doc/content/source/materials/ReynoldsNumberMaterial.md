# ReynoldsNumberMaterial

!syntax description /Materials/ReynoldsNumberMaterial

The Reynolds number $Re$ is computed as:

!equation
Re = \dfrac{\rho u D_h}{\mu}

with $\rho$ the fluid phase density, $u$ the velocity of the phase, $D_h$ the hydraulic diameter
and $\mu$ the dynamic viscosity.

This material also defines material properties for the derivatives of the Reynolds number with regards to:

- $\alpha rho A$
- $\alpha rho u A$
- $\alpha rho E A$
- $beta$ if the [!param](/Materials/ReynoldsNumberMaterial/beta) parameter is set by the user

!syntax parameters /Materials/ReynoldsNumberMaterial

!syntax inputs /Materials/ReynoldsNumberMaterial

!syntax children /Materials/ReynoldsNumberMaterial
