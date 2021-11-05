# Step 4 - Multiple submeshes

As a final step in the tensor mechanics section of this tutorial we'll set up a
pair of separate mesh blocks - two cantilevers side by side, fixed at the
bottom, that will be pushed towards each other with pressure boundary conditions.

!listing modules/tensor_mechanics/tutorials/introduction/mech_step04.i

## Input file

### `Mesh`

We again are using multiple mesh generators to set up a more complex mesh. You
could also use a meshing software, such as Cubit or Gmsh to create your geometry
and mesh - and will most definitely have to do so when modeling complex
geometries for which no specialized mesh generators are provided. It is crucial
to assign block and boundary IDs and names to your meshes so you can apply
material models and boundary conditions.

Here we are using the familiar
[`GeneratedMeshGenerator`](GeneratedMeshGenerator.md), but with a few more
options than last time. Note how we're specifying the extent of the mesh with
the [!param](/Mesh/GeneratedMeshGenerator/xmin) and
[!param](/Mesh/GeneratedMeshGenerator/xmax) parameters (we keep the default
value for [!param](/Mesh/GeneratedMeshGenerator/ymin), which is 0). We also use
the [!param](/Mesh/GeneratedMeshGenerator/bias_y) parameter to have the element
height in the mesh gradually shrink towards the top (you will see later why that
is a good idea). [!param](/Mesh/GeneratedMeshGenerator/boundary_name_prefix)
adds a prefix string to the names of the default boundaries that the
GeneratedMeshGenerator sets up (`left`, `right`, `top`, `bottom` (for >=2D),
`front`, and `back` (for 3D)). The `generated` block will create a mesh with the
boundaries `pillar1_left`, `pillar1_right`, `pillar1_top`, and `pillar1_bottom`.

The `generated2` generator will set up the second pillar and works very much
like the first, except that we are also adding the
[!param](/Mesh/GeneratedMeshGenerator/boundary_id_offset) to ensure that the
boundaries of the second pillar have their own unique IDs (this is done by
shifting the default IDs of the four boundaries by 4 so they don't overlap with
the IDs of pillar1).

!alert note
When modeling the deformation/bending of walls with solid elements make sure to
mesh at least about *five* elements through the wall thickness!

The [`MeshCollectionGenerator`](MeshCollectionGenerator.md) then combines the
two separate pillar mesh objects into a single mesh object that contains all
elements from both meshes. Note that this does **not** mean the two meshes are
glued together or connected in any way! Also, in contrast to the previous step
the mesh generators do not form a *chain*, but they form a tree structure. The
important thing for MOOSE is that a single generator sits at the end of such a
chain or root of such a tree (otherwise MOOSE will complain with an error
message).

### TensorMechanics `Master` Action

As discussed previously, when we expect large deformation we need to make sure
we select the correct strain formulation. So here we select finite strain.

### `BCs`

The boundary conditions should look familiar. Note how we can specify multiple
boundaries for every boundary condition (or Pressure action).

### `Materials`

We select
[`ComputeFiniteStrainElasticStress`](ComputeFiniteStrainElasticStress.md) as the
elastic stress calculator for the finite strain formulation.

### `Executioner`

Some more tweaks are made to the Executioner block:

- We disable [!param](/Executioner/Transient/line_search). Line searches can accelerate the convergence of the linear system, but here it does more harm than good (check for yourself by commenting out this option).
- We add a predictor to accelerate the solve. The `SimplePredictor` extrapolates an initial guess for the current time step from the two previous time steps. Especially with a Newton solve a good initial guess can lead to drastically improved convergence.

## Questions

### Expected outcome

First off, run the simulation and look at the result.

> What do you observe (does it meet your expectations)?

[Click here for the answer.](tensor_mechanics/tutorials/introduction/answer04a.md)

### Convergence

Getting a problem to converge optimally can be tricky. Try a few different things

- Remove all petsc options and check how the problem converges with the default ILU preconditioner
- comment out the `line_search` option
- remove the predictor

> Check the consequence of these changes. You can inspect internal performance
> metrics of MOOSE by adding the option `perf_graph = true` in the `[Outputs]`
> block!

### Iterative preconditioning

Check how the problem converges with the following PETSc options

```
petsc_options_iname = '-pc_type -pc_hypre_type'
petsc_options_value = 'hypre boomeramg'
```

This enables a very scalable algebraic multigrid (AMG) preconditioner, provided
by the
[Hypre](https://computing.llnl.gov/projects/hypre-scalable-linear-solvers-multigrid-methods/software)
library.

### Postprocessing results

MOOSE has a system of so called [Postprocessors](Postprocessors/index.md) that
can extract scalar quantities for a simulation while it runs. These quantities
can be printed on the screen in real time or written to files for later analysis.
You add postprocessors under the `[Postprocessors]` top level block.

File output of postprocessors is enabled by adding the `csv = true` option in
the `[Outputs]` block.

> Let's see if we can extract the x deflection of the left cantilever. It should
> be a positive value and the scalar we can extract is the maximum `disp_x`
> value in the simualtion cell (the left cantilever has positive displacements
> as it bends right in the positive x direction, while the right cantilever has
> negative displacements as it bends left in the negative x direction). So we
> need a postprocessor object that gives us an extreme value of a given
> variable. Take a look at [NodalExtremeValue](NodalExtremeValue.md) and try to
> set it up to output the maximum positive x deflection.

[Click here for the answer.](tensor_mechanics/tutorials/introduction/answer04b.md)

### Sidebar: Volumetric locking

[Volumetric locking](tensor_mechanics/VolumetricLocking.md) can occur in
mechanics simulations of near incompressible materials if the elements (and
associated shape functions) cannot accomodate the incompressibility constraint.

> Keep the postprocessing modifications from the previous question, set the
> [!param](/Materials/ComputeIsotropicElasticityTensor/poissons_ratio) of the
> cantilevers to `0.49`. Then add and modify the following parameters
>
> - [!param](/Mesh/uniform_refine) in the `[Mesh]` block
> - [!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/volumetric_locking_correction) in the tensor mechanics master action block
> - [!param](/Mesh/GeneratedMeshGenerator/elem_type) in the `GeneratedMeshGenerator` blocks
>
> For convenience all those parameters may be listed under `[GlobalParams]`
> instead. Compare first order QUAD4 elements to second order QUAD8 elements,
> compare the cantilever deflection with and without volumetric locking
> correction (with QUAD4 elements), and compare the result for different levels
> of uniform refinement (1, 2, 3, 4).

[Click here for the sidebar on volumetric locking.](tensor_mechanics/tutorials/introduction/step04a.md)

Once you've answered the questions and run this example we will move on to
[Step 1](contact/tutorials/introduction/step04.md optional=True) of the contact
tutorial, which builds on this step.
