# DebugResidualAux

!syntax description /AuxKernels/DebugResidualAux

This kernel is used for debugging. The `[Debug]` block parameter `show_var_residual_norms`
may be used to identify a problematic variable whose residual does not decrease over
the solve. The spatial dependence of this variable's contribution to the residual may
then be examined.

More information about debugging convergence issues may be found
[here](modules/doc/content/application_usage/failed_solves.md optional=True).

!syntax parameters /AuxKernels/DebugResidualAux

!syntax inputs /AuxKernels/DebugResidualAux

!syntax children /AuxKernels/DebugResidualAux
