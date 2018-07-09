# RandomIC

RandomIC produces a field of random numbers in the range specified by its "min" and "max" parameters (inclusive).
An initial seed value may be set with the "seed" parameter.

!alert note
The RandomIC class does not currently produce parallel or thread agnostic random fields (If you don't
know what this means, don't worry about it). Additionally, this class uses the global random number
generator instead of the random number system in MOOSE. It's possible that the seed value could be
clobbered by other classes using the global generator.

## Class Description

!syntax description /ICs/RandomIC

!syntax parameters /ICs/RandomIC

!syntax inputs /ICs/RandomIC

!syntax children /ICs/RandomIC

!bibtex bibliography
