# MaterialRateRealAux

!syntax description /AuxKernels/MaterialRateRealAux

## Description

The `MaterialRateRealAux` AuxKernel is used to output the rate of change of
material properties as an element-level, variable.
The rate is computed by

!equation
rate = \dfrac{\mathrm{property}(t)-\mathrm{property}(t_{old})}{dt}

where $dt=t-t_{old}$ is the time step size and [!param](/AuxKernels/MaterialRateRealAux/property) is the material property being evaluated.
This class derives from the same base class as [MaterialRealAux.md] and therefore also has optional parameters [!param](/AuxKernels/MaterialRateRealAux/factor) and [!param](/AuxKernels/MaterialRateRealAux/offset).  

## Example Input Syntax

!listing test/tests/auxkernels/material_rate_real/material_rate_real.i block=AuxKernels

!syntax parameters /AuxKernels/MaterialRateRealAux

!syntax inputs /AuxKernels/MaterialRateRealAux

!syntax children /AuxKernels/MaterialRateRealAux
