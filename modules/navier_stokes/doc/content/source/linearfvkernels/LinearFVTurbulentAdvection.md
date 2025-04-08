# LinearFVTurbulentAdvection

This object adds a $\nabla \cdot \vec u \phi$ term for an arbitrary scalar field
$\phi$, where $\phi$ corresponds to the variable that this kernel acts
on.
The [!param](/LinearFVKernels/LinearFVTurbulentAdvection/variable) can be of type `MooseLinearVariableFVReal`.

The particularity of this kernel is that it allows us to skip computing advection
for near-wall elements. The keys for this skip are the boundaries identified in
the [!param](/LinearFVKernels/LinearFVTurbulentAdvection/walls) list.
For any element that is in contact with a boundary identified
in the [!param](/LinearFVKernels/LinearFVTurbulentAdvection/walls) list,
advection will be skipped for that element over all faces.

!syntax parameters /LinearFVKernels/LinearFVTurbulentAdvection

!syntax inputs /LinearFVKernels/LinearFVTurbulentAdvection

!syntax children /LinearFVKernels/LinearFVTurbulentAdvection
