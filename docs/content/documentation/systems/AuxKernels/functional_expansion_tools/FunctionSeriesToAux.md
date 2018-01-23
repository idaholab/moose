# FunctionSeriesToAux
!syntax description /AuxKernels/FunctionSeriesToAux

This `AuxKernel` expands an FE into the named **AuxVariable** before the initial nonlinear solve. It subclasses `FunctionAux` to perform the actual work. The only differences in `FunctionSeriesToAux` are:
1) it ensures that the provided `Function` is a subclass of `FunctionSeries`
2) it ensures that it is executed at **EXEC_TIMESTEP_BEGIN**


!syntax parameters /AuxKernels/FunctionSeriesToAux

!syntax inputs /AuxKernels/FunctionSeriesToAux

!syntax children /AuxKernels/FunctionSeriesToAux
