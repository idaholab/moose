# MassKernel

This object is meant to build mass matrices for preconditioning techniques that
require them. It is only meant for filling matrices, so all vector tag
parameters are suppressed. The [!param](/Kernels/MassKernel/matrix_tags)
parameter default is cleared such that the user should provide some non-empty
parameter value for [!param](/Kernels/MassKernel/matrix_tags) or
[!param](/Kernels/MassKernel/extra_matrix_tags).

!syntax parameters /Kernels/MassKernel

!syntax inputs /Kernels/MassKernel

!syntax children /Kernels/MassKernel
