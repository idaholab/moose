# Discrete Nucleation

!image media/nucleation.gif width=50% padding-left=20px float=right caption=Nucleation system in action on a FeCuNi system. Clockwise from top left: Copper concentration, nickel concentration, nucleation penalty energy density, bulk free energy density.

The _discrete nucleation_ system allows users to incorporate nucleation phenomena in phase field simulations. Due to the lack of thermal fluctuations (also see [Langevin Noise](LangevinNoise)) nucleation phenomena are not intrinsic to the phase field method. We introduce nucleation by artificially triggering and stabilizing the formation of nuclei through local modifications of the free energy density.

The comprises two user objects and a material class:

* [`DiscreteNucleationInserter`](/UserObjects/DiscreteNucleationInserter.md) - a user object that maintains a global list of currently active nucleus positions.
* [`DiscreteNucleationMap`](/UserObjects/DiscreteNucleationMap.md)  - a user object that maintains a smooth density map for nuclei locations (obtained from a DiscreteNucleationInserter).
* [`DiscreteNucleation`](/Materials/DiscreteNucleation.md)  - a material user object that calculates a local free energy penalty based on the difference of a set of given concentration variables and their target concentrations (using the data from the DiscreteNucleationMap).

## Discussion
The nucleation free energy penalty is added to the physical free energy contributions of the system using a [`DerivativeSumMaterial`](/Materials/DerivativeSumMaterial.md)

## Example
An example input file is located at

!text modules/phase_field/examples/nucleation/cahn_hilliard.i overflow-y=scroll max-height=500px language=puppet
