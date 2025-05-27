# LinearFVTurbulentDiffusion

This object extends [`LinearFVDiffusion`](LinearFVDiffusion.md) to allow diffusion to be skipped
at certain boundaries.

The particularity of this kernel is that it allows us to skip computing diffusion
for near-wall elements. The keys for this skip are the boundaries identified in
the [!param](/LinearFVKernels/LinearFVTurbulentDiffusion/walls) list.
For any element that is in contact with a boundary identified
in the [!param](/LinearFVKernels/LinearFVTurbulentDiffusion/walls) list,
diffusion contributions will be skipped for that element over all faces.

!syntax parameters /LinearFVKernels/LinearFVTurbulentDiffusion

!syntax inputs /LinearFVKernels/LinearFVTurbulentDiffusion

!syntax children /LinearFVKernels/LinearFVTurbulentDiffusion
