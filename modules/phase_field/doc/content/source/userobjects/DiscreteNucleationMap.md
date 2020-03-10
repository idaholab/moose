# DiscreteNucleationMap

!syntax description /UserObjects/DiscreteNucleationMap

The map objects builds a smooth map of nucleation sites in the simulation
domain, expanding the (time, x, y, z, radius) description from the
[`DiscreteNucleationInserter`](/DiscreteNucleationInserter.md) to a description
in which nucleation sites have a finite size and a smooth interface. This object
takes two parameters,

- ```periodic``` - a variable to take the periodicity information of the simulation domain
- ```inserter``` - the ```DiscreteNucleationInserter``` object that maintains the nucleus list

The map object only updates the map if during a timestep the nucleus list was changed.

The `DiscreteNucleationMap` is part of the [Discrete Nucleation system](Nucleation/DiscreteNucleation.md).

!syntax parameters /UserObjects/DiscreteNucleationMap

!syntax inputs /UserObjects/DiscreteNucleationMap

!syntax children /UserObjects/DiscreteNucleationMap
