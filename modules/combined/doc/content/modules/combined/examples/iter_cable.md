# ITER Cable Electro-Thermo-Mechanics

!alert construction
This documentation is under construction and will be completed with detailed content.

## Overview

This example demonstrates coupled electro-thermo-mechanical simulations of ITER (International Thermonuclear Experimental Reactor) central solenoid superconducting cables. The simulations combine:

- Finite strain mechanics (Updated Lagrangian formulation)
- Temperature-dependent thermal expansion
- Electromagnetic Lorentz body forces (J × B)
- Multi-material composite structures

## Problem Description

[TO BE COMPLETED: Description of ITER cable structure, physics importance, engineering relevance]

### Geometry

The ITER cable consists of three distinct regions:

- **Channel**: Helium cooling channel (inner radius)
- **Bundle**: Cu-Nb3Sn composite conductor (middle annulus)
- **Jacket**: JK2LB steel structural jacket (outer square)

### Materials

[TO BE COMPLETED: Material properties, temperature ranges, composite approach]

## Input File Structure

All input files are completely independent and self-contained (~470-580 lines each). No base file inheritance.

### Shared Parameters

- **`iter_cable.params`** - Numeric parameters shared across all geometries
  - Geometry: cable_length, cable_radius, channel_radius, steel_jacket_side_length
  - Electromagnetic: current_density_z, vacuum_permeability
  - Temperature: starting_temperature (4.5 K), ending_temperature (300 K)
  - Simulation: simulation_time (10 s), refinement_level (2)
  - Included via `!include iter_cable.params` in each input file

### Geometry Variants

Four independent geometry files are provided for comparative studies:

#### copper_cylinder.i (~470 lines)
- **Geometry**: Single solid copper cylinder (no channel block)
- **Materials**: Pure OFHC copper throughout
- **Use case**: Verification case for copper properties, simplest geometry
- **Postprocessors**: Global stress averages
- **Output**: data/copper_cylinder/copper_cylinder_out.{e,csv}

#### cylinder.i (~490 lines)
- **Geometry**: Single solid Cu-Nb3Sn cylinder (no channel block)
- **Materials**: Cu-Nb3Sn effective material (2/3 Cu + 1/3 Nb3Sn weighted average)
- **Use case**: Hybrid superconductor verification, no structural jacket effects
- **Postprocessors**: Global stress averages
- **Output**: data/cylinder/cylinder_out.{e,csv}

#### annulus.i (~530 lines)
- **Geometry**: Annulus (bundle only, channel and jacket deleted)
- **Materials**: Cu-Nb3Sn effective material
- **Use case**: Isolate conductor behavior without channel or jacket geometry
- **Postprocessors**: Global and bundle-specific stress averages
- **Output**: data/annulus/annulus_out.{e,csv}

#### iter_cable.i (~580 lines)
- **Geometry**: Full cable (channel + bundle + jacket, all materials)
- **Materials**: Helium channel, Cu-Nb3Sn effective bundle, JK2LB steel jacket
- **Use case**: Complete ITER cable simulation with structural jacket
- **Postprocessors**: Global, bundle-specific, and jacket-specific stress averages
- **Output**: data/cable/iter_cable_out.{e,csv}

### Test Infrastructure

- **`tests`** - MOOSE TestHarness regression test specification
  - Exodiff tests for mesh/displacement verification
  - CSVDiff tests for postprocessor data verification
  - Heavy tests for Lorentz force verification and comparison plots
- **`gold/`** - Reference output files for regression testing
  - Run tests with: `~/projects/moose/run_tests -i examples/iter_electrothermomechanics`

## Physics Implementation

### Finite Strain Mechanics

[TO BE COMPLETED: Updated Lagrangian, logarithmic strain, displaced mesh considerations]

### Thermal Expansion

[TO BE COMPLETED: Temperature-dependent alpha(T), eigenstrain approach]

### Lorentz Forces

[TO BE COMPLETED: J × B body forces, analytical fields vs. coupled EM module]

## Material Properties

### Copper (OFHC)

Temperature-dependent properties from NIST Cryogenic Materials Database:

- Young's modulus: E(T) = 1000 × (137 - 1.27×10⁻⁴ T²) [MPa]
- Poisson's ratio: ν(T) = 0.339 + 7.03×10⁻⁸ T²
- Thermal expansion: α(T) [7th-order polynomial in log₁₀(T)]
- Valid range: 4-300 K
- Source: NIST Monograph 177

### Nb3Sn Superconductor

[TO BE COMPLETED: Properties when available]

### Cu-Nb3Sn Effective Material

Effective properties using weighted average:
- Effective material = 2/3 × Copper + 1/3 × Nb3Sn

[TO BE COMPLETED: Rationale for 2/3:1/3 ratio]

### JK2LB Steel

[TO BE COMPLETED: Temperature-dependent properties when available]

## Verification and Analysis

### Lorentz Force Verification

A Python verification script (`verify_lorentz_force_field.py`) compares MOOSE simulation results to analytical solutions:

```bash
./verify_lorentz_force_field.py --geometry copper_cylinder
./verify_lorentz_force_field.py --geometry cylinder
./verify_lorentz_force_field.py --geometry annulus
./verify_lorentz_force_field.py --geometry cable
```

The script verifies:
- Lorentz force magnitude matches analytical J × B solution
- Force direction is radially outward (hoop stress)
- Axisymmetry: force field identical at all azimuthal angles
- Axial uniformity: force field uniform along cable length

Output:
- Verification logs: `data/{geometry}/lorentz_verification.log`
- Diagnostic figures: `figures/{geometry}/lorentz_verification.png`

### Stress Comparison

A Python comparison script (`comparison_iter.py`) generates stress comparison plots across all geometries:

```bash
./comparison_iter.py [--data-dir data] [--output-dir figures/comparison]
```

Generated plots:
- **Global comparisons**: von Mises, σ_zz, σ_xx, σ_xy for all geometries
- **Block-specific comparisons**: Bundle and jacket stresses (annulus, cable)
- **Per-geometry evolution**: All stress components over time
- **Temperature verification**: Confirms identical temperature ramp

Output: `figures/comparison/*.png`

## Results

[TO BE COMPLETED: Representative results, figures, verification plots]

## References

- NIST Cryogenic Materials Database: [https://trc.nist.gov/cryogenics/materials/OFHC%20Copper/](https://trc.nist.gov/cryogenics/materials/OFHC%20Copper/)
- [TO BE COMPLETED: Additional references]

## Input Files

The complete input files for this example can be found at:

```
modules/combined/examples/iter_electrothermomechanics/
```
