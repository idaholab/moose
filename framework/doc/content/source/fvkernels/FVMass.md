# FVMass

This object is meant to build mass matrices for preconditioning techniques that
require them. It is only meant for filling matrices, so all vector tag
parameters are suppressed. The [!param](/FVKernels/FVMass/matrix_tags)
parameter default is cleared such that the user should provide some non-empty
parameter value for [!param](/FVKernels/FVMass/matrix_tags) or
[!param](/FVKernels/FVMass/extra_matrix_tags).

!syntax parameters /FVKernels/FVMass

!syntax inputs /FVKernels/FVMass

!syntax children /FVKernels/FVMass
