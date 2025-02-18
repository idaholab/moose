# PINSFVScalarFieldAdvection

This object adds a $\nabla \cdot \vec u_{superficial} \phi / \epsilon$ term for an arbitrary scalar field
$\phi$, where $\phi$ corresponds to the nonlinear variable that this kernel acts
on. The nonlinear `variable` can be of type `MooseVariableFVReal` or for
consistency with other INSFV naming conventions, can be of type
[`INSFVScalarFieldVariable`](INSFVScalarFieldVariable.md).

!syntax parameters /FVKernels/PINSFVScalarFieldAdvection

!syntax inputs /FVKernels/PINSFVScalarFieldAdvection

!syntax children /FVKernels/PINSFVScalarFieldAdvection
