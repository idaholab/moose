# DiscreteNucleationMap

!syntax description /UserObjects/DiscreteNucleationMap

The map objects builds a smooth map of nucleation sites in the simulation
domain, expanding the point description from the
[`DiscreteNucleationInserter`](/DiscreteNucleationInserter.md) to a description
in which nucleation sites have a finite size and a smooth interface. This object
takes three parameters

- ```radius``` - the radius of the area in which each nucleation site modifies the local free energy. This should be chosen sufficiently large to generate a stable precipitate, but small enough to minimize the influence on the phase field evolution
- ```periodic``` - a variable to take the periodicity information of the simulation domain
- ```inserter``` - the ```DiscreteNucleationInserter``` object that maintains the nucleus list

The map object only updates the map if during a timestep the nucleus list was changed.

The `DiscreteNucleationMap` is part of the [Discrete Nucleation system](Nucleation/DiscreteNucleation.md).

!syntax parameters /UserObjects/DiscreteNucleationMap

!syntax inputs /UserObjects/DiscreteNucleationMap

!syntax children /UserObjects/DiscreteNucleationMap
