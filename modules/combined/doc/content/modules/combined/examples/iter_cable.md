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

All geometry variants share common physics through **`cylindrical_base.i`** (~380 lines) to eliminate code duplication. Each variant defines its mesh and material overrides, then includes the base file.

### Shared Components

- **`iter_cable.params`** - Numeric parameters shared across all geometries
  - Geometry: cable_length, cable_radius, channel_radius, steel_jacket_side_length
  - Electromagnetic: current_density_z, vacuum_permeability
  - Temperature: starting_temperature (4.5 K), ending_temperature (300 K), stress_free_temperature
  - Simulation: simulation_time (10 s), refinement_level (2)
  - Included via `!include iter_cable.params` in each input file

- **`cylindrical_base.i`** - Common physics for all variants
  - Finite strain mechanics (Updated Lagrangian, logarithmic strain)
  - Temperature-dependent material properties (base component definitions)
  - Thermal expansion eigenstrain
  - Lorentz force body loads (J × B with use_displaced_mesh=true)
  - Boundary conditions (pins 3 nodes at z=0 to constrain rigid body motion)
  - Solver configuration
  - Included via `!include cylindrical_base.i` after mesh definition

### Geometry Variants

Four geometry files with shared physics via cylindrical_base.i:

#### copper_cylinder.i
- **Geometry**: Single solid copper cylinder (no channel block)
- **Materials**: Pure OFHC copper via material overrides
- **Mesh**: 10 radial rings with refinement_level=2
- **Use case**: Verification case for copper properties, simplest geometry
- **Postprocessors**: Global stress averages (from cylindrical_base.i)
- **Output**: data/copper_cylinder/copper_cylinder_out.{e,csv}

#### cylinder.i
- **Geometry**: Single solid Cu-Nb3Sn cylinder (no channel block)
- **Materials**: Cu-Nb3Sn effective material (2/3 Cu + 1/3 Nb3Sn weighted average)
- **Mesh**: 10 radial rings with refinement_level=2
- **Use case**: Hybrid superconductor verification, no structural jacket effects
- **Postprocessors**: Global stress averages (from cylindrical_base.i)
- **Output**: data/cylinder/cylinder_out.{e,csv}

#### annulus.i
- **Geometry**: Annulus (bundle only, channel deleted via BlockDeletionGenerator)
- **Materials**: Cu-Nb3Sn effective material
- **Mesh**: Channel deleted, bundle has 10 radial rings with refinement_level=2
- **Use case**: Isolate conductor behavior without channel or jacket geometry
- **Postprocessors**: Global stress averages (from cylindrical_base.i), plus bundle-specific averages
- **Output**: data/annulus/annulus_out.{e,csv}

#### iter_cable.i
- **Geometry**: Full cable (bundle + jacket, channel deleted)
- **Materials**: Cu-Nb3Sn effective bundle, JK2LB steel jacket (placeholder properties)
- **Mesh**: Channel deleted, bundle has 10 radial rings, jacket has 4 radial rings (refinement_level=2)
- **Mesh refinement**: rings='1 10 4' ensures bundle mesh matches other geometries
- **Boundary conditions**: Pins at jacket corners (not edge midpoints) to minimize constraint artifacts
- **Use case**: Complete ITER cable simulation with structural jacket
- **Postprocessors**: Global averages (from cylindrical_base.i), plus bundle and jacket-specific averages
- **Note**: Axial line sampler VectorPostprocessor currently disabled (mesh compatibility issue under investigation)
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

Features:
- Handles iter_cable.i output filename mapping (cable geometry → iter_cable_out.csv)
- Gracefully handles missing axial line sampler data (expected for iter_cable.i)

The script verifies:
- Lorentz force magnitude matches analytical J × B solution
- Force direction is radially inward (compressive hoop stress)
- Axisymmetry: force field identical at all azimuthal angles
- Axial uniformity: force field uniform along cable length (when axial line sampler is enabled)

**Note**: Analytical solution assumes infinite cylinder with uniform axial current. Validity in the rectangular steel jacket region (beyond bundle) is under investigation - relative errors remain <0.1% but increase near jacket boundaries due to geometry effects.

Output:
- Verification logs: `figures/{geometry}/lorentz_verification.log`
- Diagnostic figures: `figures/{geometry}/lorentz_verification.png`, `lorentz_0deg_error.png`

### Stress Comparison

A Python comparison script (`comparison_iter.py`) generates stress comparison plots across all geometries:

```bash
./comparison_iter.py [--data-dir data] [--output-dir figures/comparison]
```

Features:
- Automatically wipes comparison directory to remove stale plots
- Handles iter_cable.i output filename mapping (cable geometry → iter_cable_out.csv)

Generated plots:
- **Global comparisons**: von Mises, σ_zz, σ_xx, σ_xy for all geometries
- **Bundle comparisons**: Bundle stress across all 4 geometries (uses global postprocessors for single-block geometries, bundle-specific for cable)
- **Jacket-only evolution**: Jacket stress components for cable geometry (saved to figures/cable/)
- **Per-geometry evolution**: All stress components over time for each geometry
- **Temperature verification**: Confirms identical temperature ramp

Output:
- `figures/comparison/*.png` - Cross-geometry comparisons
- `figures/{geometry}/*.png` - Individual geometry plots
- `figures/cable/jacket_stress_evolution.png` - Jacket-only stress evolution

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
