# DiscreteNucleationData

!syntax description /Postprocessors/DiscreteNucleationData

The data to be extracted from the
[DiscreteNucleationInserter](/DiscreteNucleationInserter.md) is selected using
the `value` parameter, which has the following options:

|`value`|Output|
|-|-|
|`COUNT`| Number of currently active nucleation sites maintained by the inserter|
|`UPDATE`| Boolean (0/1) value denoting if at least one new nucleus was inserted or removed (i.e. its hold time expired) for the current timestep|
|`RATE`| Total nucleation rate per unit time for the entire domain|
|`INSERTIONS`| Number of new nucleation points added to the inserter's list|
|`DELETIONS` | Number of nucleation points that were removed due to their hold time having expired|

The `DiscreteNucleationData` postprocessor is part of the
[Discrete Nucleation system](Nucleation/DiscreteNucleation.md).

!syntax parameters /Postprocessors/DiscreteNucleationData

!syntax inputs /Postprocessors/DiscreteNucleationData

!syntax children /Postprocessors/DiscreteNucleationData

!bibtex bibliography
