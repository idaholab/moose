# LinearFVTurbulentLimitedDiffusion

This object extends [`LinearFVDiffusion`](LinearFVDiffusion.md) to allow diffusion to be skipped
at certain boundaries.

The particularity of this kernel is that it allows us to skip computing diffusion
for near-wall elements. The key for this skip are the boundaries identified in
the [!param](/LinearFVKernels/LinearFVTurbulentLimitedDiffusion/walls) list.
For any element that is in contact with a boundary identified
in the [!param](/LinearFVKernels/LinearFVTurbulentLimitedDiffusion/walls) list,
diffusion contributions will be skipped for that element over all faces.

!syntax parameters /LinearFVKernels/LinearFVTurbulentLimitedDiffusion

!syntax inputs /LinearFVKernels/LinearFVTurbulentLimitedDiffusion

!syntax children /LinearFVKernels/LinearFVTurbulentLimitedDiffusion
