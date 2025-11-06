# SmoothMeshGenerator

!syntax description /Mesh/SmoothMeshGenerator

The `SmoothMeshGenerator` exposes two mesh smoothing algorithms, which are described below.

## Laplace Algorithm

The Laplace smoothing algorithm utilizes a classical Laplacian smoother that iteratively relocates interior nodes to the average of their neighbors.
Boundary and subdomain boundary nodes are not moved.
This implementation currently requires a replicated mesh.

## Variational Algorithm

The variational smoothing algorithm is a Newton-based variational optimizer that minimizes a mixed *distortion–dilation* energy.
It supports 1D/2D/3D meshes, preserves exterior boundaries by constraining nodes, can optionally preserve subdomain (block) boundaries, and can *untangle some* tangled meshes before smoothing.
Internally it builds a libMesh system that assembles the gradient (residual) and analytic Hessian (jacobian) of the energy and solves to a stationary point.

#### Mathematical Formulation

The Variational Mesh Smoother minimizes a *distortion–dilation energy functional*, $I_h(R)$, originally proposed by *Branets (2005)*:

$I_h(R)=\sum_{c=1}^{N_{\text{elem}}} \int_{\hat{\Omega}_c} E_\theta(S_c(R))\, d\hat{\xi}\,,$

where:

- $R$ is the vector of nodal coordinates
- $S_c$ is the spatially-dependent jacobian matrix of the target-to-physical element mapping in element $c$
- $E_\theta$ is the spatially-dependent distortion-dilation energy density
- $\hat{\Omega}_c$ is the target element corresponding to physical element $c$
- $\hat{\xi}$ is the target space coordinate

The relationship between the target and reference elements is discussed below in the Notes & Limitations section.
Note that for each element, integration is over the target space, not the physical space.
Minimization is achieved using a damped Newton method with analytically derived gradient and Hessian of $E_\theta(S)$.

For each mesh element, the local energy density is

$E_\theta(S) = \theta\,\mu(S) + (1 - \theta)\,\beta(S) \,,$

where:

- $0 \le \theta \le 1$ is the *dilation weight* (`dilation_weight` in input)
- $\mu(S)$, penalizes *volumetric dilation* from the target element
- $\beta(S)$, penalizes *shape distortion* from the target element

The *distortion metric* is

$\beta(S)=\dfrac{\left(\tfrac{1}{n}\,\mathrm{tr}(S^{\mathsf T}S)\right)^{n/2}}{\chi_\varepsilon(\det S)}$,

and the *dilation metric* is

$\mu(S)=\dfrac{1}{2\,\chi_\varepsilon(\det S)}\left(v + \dfrac{(\det S)^2}{v}\right)$.

Definitions:

| Symbol | Meaning |
|:--|:--|
| $n$ | spatial dimension of the mesh (1, 2, or 3) |
| $\det S$ | jacobian determinant of the target-to-physical element mapping |
| $\mathrm{tr}(S^{\mathsf T}S)$ | trace of the specified matrix |
| $v$ | reference (i.e., desired) $\det S$, or "volume" for each element |
| $\chi_\varepsilon(x) = \tfrac{1}{2}(x + \sqrt{\varepsilon^2 + x^2})$ | smooth barrier ensuring well-defined metrics for degenerate or folded elements (i.e., $\det S < 0$) |
| $\varepsilon$ | small nonzero regularization constant |


#### Notes & Limitations

##### Target Elements

The target element defines the "ideal" shape for a given element type.
For some element types, the target element is the same as the reference element used by libMesh.
For other types, the target element differs from the reference element.
This is reflected in the table below.

| Element Type | Reference Element | Target Element |
|:--|:--|:--|
| EDGE | line | line |
| TRI | right triangle | equilateral triangle |
| QUAD | square | square |
| PRISM | right triangular base | equilateral triangular base, equal face areas |
| HEX | cube | cube |
| PYRAMID | square base, isosceles triangular sides  | square base, equilateral triangular sides |
| TET | right tet | regular tet |

##### Reference Volume ($v$)

The term reference "volume" is a bit misleading.
A more descriptive, albeit longer, term is "reference target-to-physical jacobian determinant".
Basically, $v$ is the jacobian determinant that we want the smoother to push our mesh elements towards.
The term "volume" is used here because the jacobian determinant is proportional to the volume of an element.
When a given mesh element has $\det S \neq v\,,$ the element is considered to be "dilated" from an element with $\det S = v\,.$

The reference volume is automatically calculated as the volumetric average of $\det S$ for each element, averaged over the number of elements in the mesh:

$v = \dfrac{1}{N_\text{elem}} \sum_{c=1}^{N_\text{elem}} \dfrac{\int_{\hat{\Omega}_c} \det S_c d\hat{\xi}}{\int_{\hat{\Omega}_c} d\hat{\xi}} \,.$

##### Element Order

The variational smoother rejects mixed-order meshes; all active elements must share the same default order.
Higher-order element types are supported.

##### Untangling

The mesh is considered tangled if, at any quadrature point, $\det S \leq 0$.
If the mesh is initially tangled, the variational smoother runs an *untangling stage* using only the distortion term (temporarily sets the dilation weight to 0), then a *smoothing stage* with the requested weights.
Not all tangled meshes can be untangled.
The barrier term $\chi_\varepsilon(x)$ behaves like $x$ for $x \gg 0$, is nonzero at $x = 0$, and asymptotes to 0 as $x$ approaches $-\infty$.
The use of this term in place of $\det S$ in denominators allows the smoother to untangle meshes with degenerate/folded elements.

Once the mesh has been untangled, $\varepsilon$ is set to zero to prevent re-tangling during the smoothing solve.

##### Optimal Solutions

It can be shown that, for the smoothing solve, when $\varepsilon = 0$ and $\chi_\varepsilon(\det S) = \det S$, the distortion metric $\beta(S)$ is minimized by any constant-scaled special orthogonal matrix such that

$S^TS = cI \,,\qquad \det S = c^n \,,\qquad \forall c > 0 \,.$

The implication of this is that the distortion metric is minimized by a mesh element that is a scaled rotation of the target element.
This makes sense because the target element is our definition of the "ideal" smooth element shape.

For untangled meshes, the dilation metric simplifies to

$\mu(S)=\dfrac{1}{2}\left(\dfrac{v}{\det S} + \dfrac{\det S}{v}\right) \,.$

It can be shown that the dilation metric is minimized by any $S$ such that $\det S = v$.

##### Boundary/Subdomain Control

Geometric constraints are automatically detected and applied to boundary nodes; if `preserve_subdomain_boundaries = true`, nodes along interfaces of differing block IDs are likewise constrained so subdomain boundaries do not drift.
Boundary/subdomain interface nodes are allowed to slide along boundaries/interfaces, so long as those surfaces are linear (e.g., a line in 2D meshes or a plane in 3D meshes).
No action is necessary to enable sliding boundary nodes as they are automatically detected.

If the surfaces are not linear, the boundary/subdomain nodes are constrained (i.e., fixed) to their original location.
It has been observed in some cases that fixing boundary nodes on nonlinear boundaries significantly restricts the space of optimally smooth solutions.
As a result, the original mesh may not change significantly.
This is because fixing boundary nodes reduces the space of possible smooth boundary elements.
These restrictions then propagate into the mesh interior.
In the future, it will be useful to allow boundary nodes to slide along nonlinear boundaries.
This is not currently possible because libMesh does not yet support nonlinear constraints.

For further theoretical background, see: Larisa V. Branets, *A Variational Grid Optimization Method Based on a Local Cell Quality Metric*, Ph.D. Dissertation, University of Texas at Austin, 2005.

## Examples

### Example 1 — Laplacian smoothing

The `iterations` parameter controls the number of smoothing steps to do.
Each smoothing step will iterate the mesh toward the “true” smoothed mesh (as measured by the Laplacian smoother).
After a few iterations the mesh typically reaches a steady state.

As an example, here is an original mesh going through 12 iterations of this smoother:

!media media/mesh/smooth.gif
       id=inl-logo
       caption=12 iterations of Laplacian smoothing.  Coloring is by element quality (higher is better).
       style=width:50%;padding:20px;

### Example 2 — Variational smoothing (fixed boundary nodes)

Here is an example of the variational smoother applied to a mesh with 3 subdomains and nonlinear boundaries.
Note that the locations of the (subdomain)boundary nodes do not change.

!listing test/tests/meshgenerators/smooth_mesh_generator/variational_mesh_smoother_generator.i block=Mesh

!media media/mesh/variational_smoother_ex1.png
       id=inl-logo
       caption=Variational smoother applied to mesh with nonlinear boundary. The original mesh is shown on the left and the smoothed mesh is shown on the right.
       style=width:50%;padding:20px;

### Example 3 — Variational smoothing (sliding boundary nodes)

Here is an example of the variational smoother applied to a mesh with 2 subdomains and linear boundaries.
Note that while the locations of the (subdomain)boundary nodes change, these boundaries are still preserved.

!listing test/tests/meshgenerators/smooth_mesh_generator/variational_mesh_smoother_generator_sliding_nodes.i block=Mesh

!media media/mesh/variational_smoother_ex3_distorted.png
       id=inl-logo
       caption=Original mesh.
       style=width:50%;padding:20px;

!media media/mesh/variational_smoother_ex3_smoothed.png
       id=inl-logo
       caption=Variationally smoothed mesh.
       style=width:50%;padding:20px;

!syntax parameters /Mesh/SmoothMeshGenerator

!syntax inputs /Mesh/SmoothMeshGenerator

!syntax children /Mesh/SmoothMeshGenerator
