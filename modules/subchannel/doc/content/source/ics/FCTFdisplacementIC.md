# FCTFdisplacementIC

!syntax description /ICs/FCTFdisplacementIC

## Overview

<!-- -->

This is a custom, hard coded kernel, that calculates the deformation of the duct (`displacement` [AuxVariable](https://mooseframework.inl.gov/syntax/AuxVariables/index.html)) for the [AREVA FCTF](https://www.osti.gov/servlets/purl/1346027/).

## Example Input File Syntax

An example input file that uses this initial condition kernel is shown below:

!listing /examples/areva_FCTF/FCTF_deformed.i language=cpp

In this input file the variable `displacement` is populated in the Initial Conditions block (ICs), using the custom kernel `FCTFdisplacementIC`

!listing /examples/areva_FCTF/FCTF_deformed.i block=ICs language=cpp

SCM models the effect of the deformation of the duct by adapting the geometric parameters of the perimetric subchannels according to a representative per subchannel deformation variable,
which is called Displacement (`displacement` D). This variable is calculated based on the centroid coordinates of each subchannel.  [!cite](KYRIAKOPOULOS2024113562).

If the

!syntax parameters /ICs/FCTFdisplacementIC

!syntax inputs /ICs/FCTFdisplacementIC

!syntax children /ICs/FCTFdisplacementIC
