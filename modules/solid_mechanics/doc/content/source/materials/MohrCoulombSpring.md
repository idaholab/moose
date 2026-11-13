# MohrCoulombSpring

!syntax description /Materials/MohrCoulombSpring

## Description

Interface spring law: local axis 1 is the normal of the interface and carries compression, and tension up to a tensile strength while the bond holds; local axes 2 and 3 carry an elastic shear whose resultant is capped by the Coulomb limit, the cohesion plus the compressive normal stress times the tangent of the friction angle, falling linearly to zero at the tensile strength. Beyond it the interface has separated, every force is zero, and the shear restarts from zero when it closes again; by default the bond is back then, with `permanent_breakage` the interface is frictional afterwards with the residual cohesion and friction angle. Without a tensile strength (the default) the interface separates at zero normal stress and the cohesion is carried at rest and until then. The law is translational only. The equations, the plastic slip and the non-symmetric tangent of the sliding state are on [SpringElements](/SpringElements.md).

### Choosing the parameters

- `normal_stiffness` and `shear_stiffness` are the elastic stiffnesses of the interface, with the dimension force per length, or force per length cubed (a subgrade modulus) when `tributary_area` is given. Axis 1 of [ComputeSpringDeformation](/ComputeSpringDeformation.md) must be the normal of the interface, pointing from node 0 to node 1 or along `local_axis_1`; node 0 moving toward node 1 closes the interface.
- `cohesion` and `friction_angle` are the strength parameters of the interface, the cohesion with the dimension force or force per area when the area is given, the angle in degrees. A reduced interface strength is written directly into them.
- `initial_normal_stress` is the compressive stress on the interface before any deformation, the dimension of the cohesion. It enters the Coulomb limit only, never the normal force of the spring (the soil carries it, not the spring), so a one-sided spring with it is in equilibrium at zero deformation; an opening relaxes it down to separation. Without it the shear capacity at zero closure is the cohesion alone. On a pile shaft it is the horizontal effective stress, which varies with depth: one law per block of springs, or a mesh whose blocks follow the depth.
- `tensile_strength` makes the interface a bonded one, a grouted anchor or a cemented contact: it carries tension up to that stress, and the shear limit falls from the cohesion to zero there, so that the bond breaks in shear and in tension together and the solve sees no jump in the shear capacity at the separation. Softening of this kind can need smaller steps than the elastic stages when many springs break at once, as with any brittle law.
- `permanent_breakage` with `residual_cohesion` (zero by default) and `residual_friction_angle` (the friction angle by default) describe the interface after a separation: a bond that does not re-form, a smooth sliding surface left by the first slip. Without the flag the bond is back on closing, which suits a frictional contact that was never bonded (the default, where the cohesion is an adhesion), and the residual parameters are refused.
- `tributary_area` is the area of the interface each spring represents; `base_name` as on [ComputeSpringDeformation](/ComputeSpringDeformation.md).

!listing modules/solid_mechanics/test/tests/spring/static/mohr_coulomb.i block=Materials

!media media/solid_mechanics/spring_law_mohr_coulomb.png
      style=width:90%;margin-left:auto;margin-right:auto;
      id=fig_mohr_coulomb_law
      caption=The normal law, the shear law and the Coulomb limit of the interface.

!syntax parameters /Materials/MohrCoulombSpring

!syntax inputs /Materials/MohrCoulombSpring

!syntax children /Materials/MohrCoulombSpring

!bibtex bibliography
