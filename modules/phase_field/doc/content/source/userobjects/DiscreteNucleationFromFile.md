# DiscreteNucleationFromFile

!syntax description /UserObjects/DiscreteNucleationFromFile

This inserter can be used for verification and test cases. Instead of
stochastic nucleus insertion via a probability calculated in
[`DiscreteNucleationInserter`](/DiscreteNucleationInserter.md), this object reads
a CSV file with columns for nucleus insertion time (`time`) and the coordinates
of the insertion point (`x`, `y`, `z`).  The nuclei inserted can be of a fixed
radius or have a variable radius.  If the parameter `radius`, which must be greater than 0, is specified, a fixed radius will be used.  If
the radius is not fixed, an additional column must be supplied in the CSV file,
such that the columns supply (`time`, `x`, `y`, `z`, `r`).  Note that the radius can be supplied in the CSV file and overridden by supplying a value for `radius`.

The `DiscreteNucleationFromFile` is part of the [Discrete Nucleation system](Nucleation/DiscreteNucleation.md).

!syntax parameters /UserObjects/DiscreteNucleationFromFile

!syntax inputs /UserObjects/DiscreteNucleationFromFile

!syntax children /UserObjects/DiscreteNucleationFromFile
