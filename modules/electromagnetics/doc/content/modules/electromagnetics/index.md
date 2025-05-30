# Electromagnetics Module

The electromagnetics module provides components and models to simulate electromagnetic
wave problems within the MOOSE framework, and facilitates multiphysics coupling of
electromagnetic simulations to other physical domains. Features include:

- Transient and time-harmonic (i.e., single-frequency, steady-state) simulation
  in 1D and 2D (3D is currently in development)
- Component-wise (scalar variables) and vector field (vector variables) components
  for the Helmholtz wave form of Maxwell's Equations
- Complex field calculations
- First-order port boundary conditions (scalar and vector forms)
- Electrostatic contact interface conditions based on the work of [!citep](cincotti2007sps)
- Parallel and perpendicular field interface conditions for electric field based on Maxwell's Equations
- Current density and electric field calculation based on electrostatic potential
- Fundamental eigenvalue solutions for 2D waveguide profiles
- Reflection coefficient calculation for a 1D slab

!media gallery/dipole_antenna.mp4
       style=width:50%;margin:auto;
       id=dipole-results-movie
       caption=Electric field radiation pattern of half-wave dipole antenna driven by a 1GHz signal, simulated using the electromagnetics module. Note that field intensity is normalized to 1.0.

- [Systems](modules/electromagnetics/systems.md) - A complete summary of all electromagnetic module objects

## Benchmarks

- [1D Slab Reflection](benchmarks/OneDReflection.md) - Calculating the reflection
  coefficient of a 1D metal-backed dielectric slab
- [Waveguide Transmission](benchmarks/WaveguideTransmission.md) - Complex field
  pattern in a single-frequency waveguide
- [Evanescent Wave Decay](benchmarks/EvanescentWave.md) - Characteristic decay and
  reflection of an evanescent wave when an obstruction is encountered
- [Half-wave Dipole Antenna](benchmarks/DipoleAntenna.md) - Radiation pattern of a
  half-wave dipole antenna excited at 1GHz (time domain result shown in [dipole-results-movie])
- [Waveguide Fundamental Eigenvalue](benchmarks/WaveguideEigenvalue.md) - Fundamental
  wave number (eigenvalue) of a 2D waveguide profile solved using [SLEPc]

## Verification Problems

- [Electrostatic Contact, Two Blocks](verification/electrostatic_contact_two_block.md) - Verification
  of electrostatic contact between a driven/powered block and a grounded block
- [Electrostatic Contact, Three Blocks](verification/electrostatic_contact_three_block.md) - Verification
  of electrostatic contact with a floating block sandwiched between a driven/powered block and a grounded
  block

## Multiphysics Coupling Cases

- [Coupling electromagnetics and heat transfer modules for wire heating](combined/examples/current_heating_of_wire.md optional=true) - Example of heating a copper wire through Joule heating by suppling the fusing current

## Citing

!! em-citation-start

This paper documents the initial development, function, verification, and validation
of the electromagnetics module.

```
@article{icenhour2024electromagnetics,
  author = {Casey T. Icenhour and Alexander D. Lindsay and Cody J. Permann and Richard C. Martineau and David L. Green and Steven C. Shannon},
  title = {The MOOSE electromagnetics module},
  journal = {SoftwareX},
  publisher = {Elsevier},
  volume = {25},
  pages = {101621},
  year = {2024},
  month = {February},
  issn = {2352-7110},
  doi = {https://doi.org/10.1016/j.softx.2023.101621},
  url = {https://www.sciencedirect.com/science/article/pii/S2352711023003175}
}
```

!! em-citation-end

While the above paper is the main citation, the following PhD dissertation documents the initial
development and capabilities of the electromagnetics module in greater detail and could be used
alongside the paper above if module capabilities are used in the to-be-published work that are not
covered therein.

```
@phdthesis{icenhourPHDelectromagnetics,
  author = {Icenhour, Casey T.},
  title = {Development and Validation of Open Source Software for Electromagnetics Simulation and Multiphysics Coupling},
  school = {North Carolina State University},
  year = {2023},
  url = {https://www.lib.ncsu.edu/resolver/1840.20/40985}
}
```
