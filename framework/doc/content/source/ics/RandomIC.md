# RandomIC

RandomIC initializes a variable using randomly generated numbers. These can either
follow a uniform distribution over a user-defined range (using the `min` and `max`
parameters), or follow an arbitrary distribution defined by a `Distribution` object
specified using the `distribution` parameter.  An initial seed value may be set with
the "seed" parameter. The RandomIC object produces a parallel agnostic random field.

!alert note The results are not currently parallel agnostic when using DistributedMesh.

## Class Description

!syntax description /ICs/RandomIC

!syntax parameters /ICs/RandomIC

!syntax inputs /ICs/RandomIC

!syntax children /ICs/RandomIC

!bibtex bibliography
