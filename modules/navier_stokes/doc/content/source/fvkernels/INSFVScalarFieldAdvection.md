# INSFVScalarFieldAdvection

This object adds a $\nabla \cdot \vec u \phi$ term for an arbitrary scalar field
$\phi$. $\phi$ can be set to a material property using the
`advected_quantity` `MaterialPropertyName` parameter. If no value for that
parameter is provided in the input file, then the associated nonlinear
`variable` is used as the advected quantity. The nonlinear `variable` can be of
type `MooseVariableFVReal` or for consistency with other INSFV naming
conventions, can be of type [`INSFVScalarFieldVariable`](INSFVScalarFieldVariable.md).

!syntax parameters /FVKernels/INSFVScalarFieldAdvection

!syntax inputs /FVKernels/INSFVScalarFieldAdvection

!syntax children /FVKernels/INSFVScalarFieldAdvection
