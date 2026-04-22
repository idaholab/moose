# ViewFactorBase

`ViewFactorBase` objects are used to determine and store view factors $F_{i,j}$, which are defined as the fraction of radiation leaving surface $i$ that is intercepted by surface $j$.

These objects are used in particular by [GrayDiffuseRadiation](/GrayDiffuseRadiation/index.md).

## Properties

Note some basic properties of view factors, denoting the surface areas of a surface $i$ as $A_i$:

- *The reciprocity relation*, valid for all surfaces:

  !equation
  A_i F_{i,j} = A_j F_{j,i}

- *The summation rule*, valid for surfaces forming an enclosure:

  !equation
  \sum\limits_j F_{i,j} = 1
