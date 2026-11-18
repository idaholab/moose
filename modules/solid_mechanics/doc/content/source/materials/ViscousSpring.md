# ViscousSpring

!syntax description /Materials/ViscousSpring

## Description

Viscous spring law, a dashpot: the force along each local axis is a damping coefficient times the rate of the deformation along that axis, and the tangent is the coefficient times the derivative of the rate with respect to the displacement of the time integrator, see [SpringElements](/SpringElements.md). The rates come from [ComputeSpringDeformation](/ComputeSpringDeformation.md) with `compute_rates = true`, so the law needs a transient executioner.

### Choosing the parameters

- `damping` holds the three coefficients in the order of the local axes, dimension force times time per length (moment times time per radian in the rotational mode), per unit area when `tributary_area` is given.
- A dashpot in parallel with a spring on the same element takes its own `base_name` on its deformation material, law and kernels, as the listing shows.
- `mode` and `tributary_area` as for [LinearSpring](/LinearSpring.md).

!listing modules/solid_mechanics/test/tests/spring/static/kelvin_voigt.i block=Materials Kernels

!media media/solid_mechanics/spring_law_viscous.png
      style=width:60%;margin-left:auto;margin-right:auto;
      id=fig_viscous_law
      caption=The viscous law.

!syntax parameters /Materials/ViscousSpring

!syntax inputs /Materials/ViscousSpring

!syntax children /Materials/ViscousSpring

!bibtex bibliography
