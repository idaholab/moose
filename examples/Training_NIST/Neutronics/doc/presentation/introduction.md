# Introduction

### Flexible, extensible reactor physics using the MOOSE framework

!media microreactor_transient.png style=width:50%;margin-left:auto;margin-right:auto;display:block;

!media griffin_joint.png style=width:70%;margin-left:auto;margin-right:auto;display:block;background-color:white;

!---

# Capabilities

- Steady-state and transient simulation for:

  - neutrons
  - photons
  - thermal radiation
  - phonons

- Various discretization schemes with multi-scheme

  - DFEM-SN
  - CFEM-Diffusion
  - HFEM-PN

- Homogenization equivalence
- Microscopic and macroscopic depletion and decay heat calculations
- Self-shielding and cross section generation
- Multiphysics coupling with heat-conduction, thermal-fluids, and thermo-mechanics

!---

### Griffin is meant be agnostic to reactor-type, but more specific capabilities exist for certain reactor types

!media reactor_examples.png

!---

# Multi-Fidelity Neutronics

!media multifidelity.png

!---

# Monte Carlo

- Exact representation of geometry and continuous energy
- Limited multiphysics and transient capabilities
- Benchmark for criticality calculations, depletion, and cross section generation

!media monte_carlo.png style=width:40%;margin-left:auto;margin-right:auto;display:block;

!---

# Heterogeneous

- Introduces energy, angular, and spatial discretization error
- Full spatial profile of flux
- Benchmark for multiphysics k-eigenvalue, depletion, and transient (and kinetics parameter generation)

!media heterogeneous.png style=width:40%;margin-left:auto;margin-right:auto;display:block;

!---

# Homogeneous

- Additional discretization error – extra steps for accurate cross sections
- Power reconstruction can retrieve fine spatial details
- Useful for design calculations, accident analysis, and uncertainty quantification

!media homogeneous.png style=width:40%;margin-left:auto;margin-right:auto;display:block;

!---

# Point Kinetics

- Significant assumptions on spatial profile (0D with fixed shape)
- Calculates general temporal behavior (low reliability for local values)
- Useful for preliminary design analysis and optimization

!media point_kinetics.png style=width:40%;margin-left:auto;margin-right:auto;display:block;
