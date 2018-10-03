# DiscreteNucleationInserter

!syntax description /UserObjects/DiscreteNucleationInserter

The inserter manages the global list of currently active nucleus stabilization sites. This user object takes two parameters

- `hold_time` - the duration in time for which a stabilization site remains active
- `probability` - a material property containing a nucleation rate density. This material property can be calculated using classical nucleation theory for example.

The inserter object keeps track if any changes to the nucleus list occurred in the current timestep.

The `DiscreteNucleationInserter` is part of the [Discrete Nucleation system](Nucleation/DiscreteNucleation.md).

!syntax parameters /UserObjects/DiscreteNucleationInserter

!syntax inputs /UserObjects/DiscreteNucleationInserter

!syntax children /UserObjects/DiscreteNucleationInserter
