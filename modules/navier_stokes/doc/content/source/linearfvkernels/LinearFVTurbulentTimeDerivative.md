# LinearFVTurbulentTimeDerivative

This object extends [`LinearFVTimeDerivative`](LinearFVTimeDerivative.md) to allow the time
derivative to be skipped at certain boundaries, in support of the standard wall functions
in the $k-\epsilon$ turbulence model.

The particularity of this kernel is that it allows us to skip computing the time derivative
for near-wall elements.
For any element that is in contact with a boundary identified
in the [!param](/LinearFVKernels/LinearFVTurbulentTimeDerivative/walls) list,
the time derivative contributions will be skipped for that element.
This is consistent with [`LinearFVTurbulentAdvection`](LinearFVTurbulentAdvection.md) and
[`LinearFVTurbulentDiffusion`](LinearFVTurbulentDiffusion.md), which skip the same elements,
leaving their rows to the wall treatment of the source kernels.

!syntax parameters /LinearFVKernels/LinearFVTurbulentTimeDerivative

!syntax inputs /LinearFVKernels/LinearFVTurbulentTimeDerivative

!syntax children /LinearFVKernels/LinearFVTurbulentTimeDerivative
