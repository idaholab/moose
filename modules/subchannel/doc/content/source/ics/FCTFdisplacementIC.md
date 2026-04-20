# FCTFdisplacementIC

!syntax description /ICs/FCTFdisplacementIC

## Overview

!! Intentional comment to provide extra spacing

This is a custom, hard-coded IC, that is used only in one specific geometry. This kernel calculates and initializes, the deformation of the duct (`displacement` [AuxVariable](https://mooseframework.inl.gov/syntax/AuxVariables/index.html)) for the [AREVA FCTF](https://www.osti.gov/servlets/purl/1346027/).
SCM models the effect of the duct deformation, by adapting the geometric parameters of the perimetric subchannels according to a representative per subchannel deformation variable, which is called Displacement (`displacement` D). This auxiliary variable is calculated based on the centroid coordinates of each subchannel [!cite](kyriakopoulos2024validation). Alternatively, the user can opt to create custom ICs to directly define the geometric parameters: surface area and wetted perimeter for the deformed edge/corner subchannels.

If not otherwise defined the default value of auxvariables like `displacement` is zero, which means that the duct is not deformed. Before each time-step, the solver checks the value of `displacement` in the perimeter subchannels and the value of `Dpin` for all pins. If there is a non-default value in any of these auxvariables, the geometric parameters of the subchannels are re-calculated before the start of the solution algorithm. The geometric parameters (surface area, wetted perimeter, gap) are depended on the auxiliary variables `Dpin` and `displacement`. It is only the displacement of the perimetric subchannels that is taken into consideration, since the perimetric subchannels are the ones next to the deformed duct.

## Example Input File Syntax

!! Intentional comment to provide extra spacing

An example input file that uses this initial condition kernel is shown below:

!listing /validation/areva_FCTF/FCTF_deformed.i language=moose

In this input file the variable `displacement` is populated in the Initial Conditions block (ICs), using the custom kernel `FCTFdisplacementIC`.

!listing /validation/areva_FCTF/FCTF_deformed.i block=ICs language=moose

!syntax parameters /ICs/FCTFdisplacementIC

!syntax inputs /ICs/FCTFdisplacementIC

!syntax children /ICs/FCTFdisplacementIC
