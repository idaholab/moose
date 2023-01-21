# Tensor Mechanics / BISON Overview

!---

## Fuel Behavior: Introduction

!row!
!col! width=25%
At beginning of life, a fuel element is quite simple...

!media fuel_schematic.png style=width:55%;

!style fontsize=50% halign=center
Nakajima et al., Nuc. Eng. Des., +148+, 41 (1994)

!col-end!

!col! width=10%
!col width=10%

$~$

$~$

$~$

but irradiation brings about substantial complexity...

!style fontsize=150%
!equation
\Longrightarrow


!col-end!

!col! width=30%
!col width=30%

!media cracked_fuel.png style=width:80%;

!style halign=center fontsize=50%
Michel et al., Eng. Frac. Mech., +75+, 3581 (2008)

!style halign=center fontsize=70%
+Fuel Fracture+

!media pellet_clad_contact.png style=width:50%

!style halign=center fontsize=50%
Olander, p. 584 (1978)

!style halign=center fontsize=70%
+Multidimensional Contact \\
and Deformation+


!col-end!

!col! width=30%
!col width=30%
!media fission_gas.png style=width:80%;display:block

!style halign=center fontsize=50%
Olander, p. 584 (1978)

!style halign=center fontsize=70%
+Fission Gas+

!media scc_failure.png style=width:80%;margin-left:auto;margin-right:auto;display:block;

!style halign=center fontsize=50%
Bentejac et al., PCI Seminar (2004)

!style halign=center fontsize=70%
+Stress Corrosion \\
Cracking Cladding \\
Failure+

!col-end!

!row-end!

!---

## Fuel Behavior Modeling: Coupled Multiphysics

!row!
!col! width=35%

!style! fontsize=80%
Multiphysics

- Fully coupled nonlinear thermo-mechanics
- Multiple species diffusion
- Neutronics
- Thermal-hydraulics
- Chemistry

Multi-space scale

- Important physics at the atomistic and micro-structural levels
- Practical engineering simulations require the continuum level

Multi-time scale

- Steady operation ($\Delta t > 1$ week)
- Power ramps/accidents \\
  ($\Delta t < 0.1$ s)
!style-end!

!col-end!

!col width=65%
!media lassmann_multi_physics.png style=margin-left:auto;margin-right:auto;display:block;

!row-end!

!---

## BISON - Nuclear Fuel Performance Analysis

- BISON is a nuclear fuel performance analysis tool. It has been used for UO$_x$ fuel but has also been used to model TRISO fuel, rod and plate metal fuel, and accident tolerant fuel.

- BISON is built on top of MOOSE, it mainly uses the [Tensor Mechanics](https://mooseframework.inl.gov/modules/tensor_mechanics/) and [Heat Conduction](https://mooseframework.inl.gov/modules/heat_conduction/index.html)modules.

- BISON is implicitL $\rightarrow$ *Large time steps*

- BISON runs in parallel $\rightarrow$ *Runs naturally on one or many processors*

- BISON is fully coupled $\rightarrow$ *(i) No operator splitting or staggered scheme necessary* + *(ii) All unknowns are solved for simultaneously*

- BISON is continuously under development!

