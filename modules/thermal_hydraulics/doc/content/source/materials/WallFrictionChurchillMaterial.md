# WallFrictionChurchillMaterial

!syntax description /Materials/WallFrictionChurchillMaterial

The Churchill friction factor material is computed with the following equations:

First the Reynold number is computed, then limited to a minimum value of 10.

!equation
\tilde{Re} = max(Re, 10.0)

Then two intermediate quantities are computed as:

!equation
a = \left(2.457 \log\left(\dfrac{1.0}{(7.0 / \tilde{Re})^{0.9}} + 0.27 \dfrac{roughness}{D_h}\right)\right)^{16}

equation
b = \dfrac{3.753e4}{\tilde{Re}}^{16}

with $D_h$ the hydraulic diameter and the roughness of the pipe being a user input.

Finally the Darcy friction factor $D_C$ for the Churchill model is computed as:

!equation
D_C = 8.0 \left( (8.0 / \tilde{Re})^{12} + \dfrac{1.0}{ (a + b)^{1.5}} \right)^{\dfrac{1.0}{12.0}}

This material also defines material properties for the derivatives of the friction factor with regards to:

- $\alpha \rho A$
- $\alpha \rho u A$
- $\alpha \rho E A$

!syntax parameters /Materials/WallFrictionChurchillMaterial

!syntax inputs /Materials/WallFrictionChurchillMaterial

!syntax children /Materials/WallFrictionChurchillMaterial
