# LinearFVDiffusionKernel

## Description

This kernel contributes to the system matrix and the right hand side
(when boundary conditions are used) of a system which is solved for a
linear finite volume variable [MooseLinearVariableFV.md].
The contributions can be derived using the numerical integral of the approximated diffusive flux
on the faces of the cells:

!equation
-\int\limits_S D(\vec{r})\nabla u(\vec{r}) \cdot \vec{n} d\vec{r} \approx
-D(\vec{r_f})\left(|\vec{\Delta}|[u(\vec{r}_N) - u(\vec{r}_C)] + \bar{\nabla u} \cdot \vec{k}\right) |S_f|,

where $D$ is a space dependent diffusion coefficient and $|S_f|$ denotes the surface area.
Vectors $\vec{\Delta}$ and $\vec{k}$ are determined to respect $\vec{n} = \vec{\Delta} + \vec{k}$
where $\vec{\Delta}$ is always parallel to the line connecting the current and neighbor cell centroids.
As shown above, using these two vectors, the approximate form of the normal-gradient is typically split into two terms:

- $|\vec{\Delta}|[u(\vec{r}_N) - u(\vec{r}_C)]$ which describes a contribution that comes from
  a finite difference approximation of the gradient on orthogonal grids.
  Hence, it is referred to as an orthogonal contribution. For orthogonal meshes, $|\Delta|$
  is just $\frac{1}{|\vec{r}_{CN}|}$ where $|\vec{r}_{CN}|$ is the distance between the
  current and neighbor cell centroids. This term contributes a ($|\vec{\Delta}||S_f|$) to the diagonal and off-diagonal entries of the system matrix with different signs.
- On non-orthogonal meshes a correction is needed: $\bar{\nabla u} \cdot \vec{k}$, where
  $\bar{\nabla u}$ denotes the interpolated gradient at the cell center computed by the cell
  gradients on the current and neighbor cells. This term is treated in an explicit manner
  meaning that it is added to the right hand side vector of the system.

For more information on the numerical representation of the diffusion term and the different
techniques used for applying boundary conditions through this kernel, see [!cite](moukkalled2016finite).

The diffusion coefficient parameter ([!param](/LinearFVKernels/LinearFVDiffusionKernel/diffusion_coeff))
accepts anything that supports functor-based evaluations. For more information on functors in
MOOSE, see [Functors/index.md].

## Example input syntax

!listing test/tests/fvkernels/fixmeee/1d.i

!syntax parameters /LinearFVKernels/LinearFVDiffusionKernel

!syntax inputs /LinearFVKernels/LinearFVDiffusionKernel

!syntax children /LinearFVKernels/LinearFVDiffusionKernel
