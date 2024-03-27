# LinearFVDiffusion

## Description

This kernel contributes to the system matrix and the right hand side of a system
which is solved for a linear finite volume variable [MooseLinearVariableFV.md].
The contributions can be derived using the integral of the diffusion term in the following form:

!equation
-\int\limits_{V_C} \nabla \cdot D \nabla u dV =
\sum\limits_f \int\limits_{S_f} -D \nabla u \cdot \vec{n} dS,

where we used the divergence theorem to transform a volumetric integral over cell $V_C$ of a
vector field to a sum of surface integrals over the faces of the cell.
Furthermore, $D$ denotes a space dependent diffusion coefficient.
With this, we can use the finite volume approximation for the face integrals in the following way:

!equation
-\int\limits_{S_f} D\nabla u \cdot \vec{n} dS \approx
-D_f\left(|\vec{\Delta}|[u_N - u_C] + \overline{\nabla u} \cdot \vec{k}\right) |S_f|,

where $|S_f|$ denotes the surface area.
Vectors $\vec{\Delta}$ and $\vec{k}$ are determined to respect $\vec{n} = \vec{\Delta} + \vec{k}$,
where $\vec{\Delta}$ is always parallel to the line connecting the current and neighbor cell centroids.
We use the over-relaxed approach for the split of the normal vector,
described in [!cite](moukalled2016finite) and [!cite](jasak1996error) in detail.
As shown above, using these two vectors, the approximate form of the normal-gradient is
typically split into two terms:

- $|\vec{\Delta}|[u_N - u_C]$ which describes a contribution that comes from
  a finite difference approximation of the gradient on orthogonal grids.
  Hence, it is referred to as an orthogonal contribution. For orthogonal meshes, $|\Delta|$
  is just $\frac{1}{|\vec{r}_{CN}|}$ where $|\vec{r}_{CN}|$ is the distance between the
  current and neighbor cell centroids. This term contributes a ($|\vec{\Delta}||S_f|$)
  to the diagonal and off-diagonal entries of the system matrix with different signs.
- On non-orthogonal meshes, besides $|\vec{\Delta}| = \frac{1}{|\vec{r}_{CN}*\vec{n}|}$,
  the following correction term is needed: $\overline{\nabla u} \cdot \vec{k}$, where
  $\overline{\nabla u}$ denotes the interpolated gradient at the face center computed using the cell
  gradients on the current and neighbor cells. This term is treated in an explicit manner
  meaning that it is added to the right hand side vector of the system.

For more information on the numerical representation of the diffusion term and the different
techniques used for applying boundary conditions through this kernel, see [!cite](moukalled2016finite)
and [!cite](jasak1996error).

The diffusion coefficient parameter ([!param](/LinearFVKernels/LinearFVDiffusion/diffusion_coeff))
accepts anything that supports functor-based evaluations. For more information on functors in
MOOSE, see [Functors/index.md].

## Example input syntax

The input file below shows a pure diffusion problem on a two-dimensional domain.

!listing test/tests/linearfvkernels/diffusion/diffusion-2d.i block=LinearFVKernels

!syntax parameters /LinearFVKernels/LinearFVDiffusion

!syntax inputs /LinearFVKernels/LinearFVDiffusion

!syntax children /LinearFVKernels/LinearFVDiffusion
