# Discrete Nucleation

!media phase_field/nucleation.gif style=width:50%;margin-left:20px;float:right;
       caption=Nucleation system in action on a FeCuNi system. Clockwise from top left: Copper
               concentration, nickel concentration, nucleation penalty energy density, bulk free
               energy density.

The _discrete nucleation_ system allows users to incorporate nucleation
phenomena in phase field simulations. Due to the lack of thermal fluctuations
(also see [Langevin Noise](Nucleation/LangevinNoise.md)) nucleation phenomena
are not intrinsic to the phase field method. We introduce nucleation by
artificially triggering and stabilizing the formation of nuclei through local
modifications of the free energy density.

The system comprises two user objects, a material class, a marker for mesh
adaptivity, and a postprocessor:

- [`DiscreteNucleationInserter`](/DiscreteNucleationInserter.md) - a user object
  that maintains a global list of currently active nucleus positions.
- [`DiscreteNucleationMap`](/DiscreteNucleationMap.md)  - a user object that
  maintains a smooth density map for nuclei locations (obtained from a
  DiscreteNucleationInserter).
- [`DiscreteNucleation`](materials/DiscreteNucleation.md)  - a material that calculates
  a local free energy penalty based on the difference of a set of given concentration
  variables and their target concentrations (using the data from the DiscreteNucleationMap).
- [`DiscreteNucleationMarker`](markers/DiscreteNucleationMarker.md)  - a marker
  that triggers refinement at the point of nucleus insertion.
- [`DiscreteNucleationTimeStep`](postprocessors/DiscreteNucleationTimeStep.md) - a
  postprocessor to provide a time step limit for new nuclei to use with
  [IterationAdaptiveDT](/IterationAdaptiveDT.md)

## Discussion

The nucleation free energy penalty is added to the physical free energy
contributions of the system using a
[`DerivativeSumMaterial`](/DerivativeSumMaterial.md)

## Example

An example input file is located at

!listing modules/phase_field/examples/nucleation/refine.i
