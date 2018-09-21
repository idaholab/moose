# DiscreteNucleationData

!syntax description /Postprocessors/DiscreteNucleationData

Outputs the number of currently active nucleation sites maintained by the given
[DiscreteNucleationInserter](/DiscreteNucleationInserter.md) (`value` option
`COUNT`), a boolean (0/1) value denoting if at least one new nucleus was
inserted or removed (i.e. its hold time expired) for the current timestep
(`value` option `UPDATE`), or the total nucleation rate for the entire domain
(`value` option `RATE`).

The `DiscreteNucleationData` postprocessor is part of the
[Discrete Nucleation system](Nucleation/DiscreteNucleation.md).

!syntax parameters /Postprocessors/DiscreteNucleationData

!syntax inputs /Postprocessors/DiscreteNucleationData

!syntax children /Postprocessors/DiscreteNucleationData

!bibtex bibliography
