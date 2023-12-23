# INSFVTurbulentAdvection

This object adds a $\nabla \cdot \vec u \phi$ term for an arbitrary scalar field
$\phi$, where $\phi$ corresponds to the nonlinear variable that this kernel acts
on. The nonlinear `variable` can be of type `MooseVariableFVReal` or for
consistency with other INSFV naming conventions, can be of type
[`INSFVEnergyVariable`](INSFVEnergyVariable.md).

The particularity of this kernel is that it allows us to skip computing advection
for near-wall elements. The key for this skip are the boundaries identified in
the [!param](/FVKernels/INSFVTurbulentAdvection/walls) list.
For any element that is in contact with a boundary identified
in the [!param](/FVKernels/INSFVTurbulentAdvection/walls) list,
advection will be skipped for that element over all faces.

!alert note
This kernel is mainly used for applying wall functions to turbulent problems.

!syntax parameters /FVKernels/INSFVTurbulentAdvection

!syntax inputs /FVKernels/INSFVTurbulentAdvection

!syntax children /FVKernels/INSFVTurbulentAdvection
