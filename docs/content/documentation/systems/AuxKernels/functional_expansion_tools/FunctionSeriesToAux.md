# Function Series to Aux

!syntax description /AuxKernels/FunctionSeriesToAux

## Description

This `AuxKernel` expands an FX into the named **AuxVariable** before the initial nonlinear solve. It subclasses `FunctionAux` to perform the actual work. The only differences in `FunctionSeriesToAux` are:
1) it ensures that the provided `Function` is a subclass of `FunctionSeries`
2) it ensures that it is executed at **EXEC_TIMESTEP_BEGIN**

## Example Input File Syntax

!listing modules/functional_expansion_tools/examples/1D_volumetric_Cartesian/main.i block=AuxKernels id=input caption=Example use of FunctionSeriesToAux

!syntax parameters /AuxKernels/FunctionSeriesToAux

!syntax inputs /AuxKernels/FunctionSeriesToAux

!syntax children /AuxKernels/FunctionSeriesToAux
