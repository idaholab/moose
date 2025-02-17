# FCTFdisplacementIC

!syntax description /ICs/FCTFdisplacementIC

## Overview

<!-- -->

This is a custom, hard-coded kernel, that is used only in one specific geometry. This kernel calculates and initializes, the deformation of the duct (`displacement` [AuxVariable](https://mooseframework.inl.gov/syntax/AuxVariables/index.html)) for the [AREVA FCTF](https://www.osti.gov/servlets/purl/1346027/).
SCM models the effect of the duct deformation, by adapting the geometric parameters of the perimetric subchannels according to a representative per subchannel deformation variable,
which is called Displacement (`displacement` D). This auxiliary variable is calculated based on the centroid coordinates of each subchannel [!cite](KYRIAKOPOULOS2024113562).

If the flag that enables deformation modeling is activated ([!param](/Problem/QuadSubChannel1PhaseProblem/deformation),[!param](/Problem/TriSubChannel1PhaseProblem/deformation)), the geometric parameters of the subchannels are re-calculated before the start of the solution algorithm.
The geometric parameters (surface area, wetted perimeter, gap) are depended on the auxiliary variables `Dpin` and `displacement`. It is only the displacement of the perimetric
subchannels that is taken into consideration, since the perimetric subchannels are the ones next to the deformed duct.

## Example Input File Syntax

An example input file that uses this initial condition kernel is shown below:

!listing /examples/areva_FCTF/FCTF_deformed.i language=cpp

In this input file the variable `displacement` is populated in the Initial Conditions block (ICs), using the custom kernel `FCTFdisplacementIC`.

!listing /examples/areva_FCTF/FCTF_deformed.i block=ICs language=cpp

!syntax parameters /ICs/FCTFdisplacementIC

!syntax inputs /ICs/FCTFdisplacementIC

!syntax children /ICs/FCTFdisplacementIC
