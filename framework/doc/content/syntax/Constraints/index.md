# Constraints System

## MortarConstraints

The mortar system in MOOSE uses a segment-based approach for evaluation of mortar integrals; for information on the automatic generation of mortar segment meshes see [AutomaticMortarGeneration](https://github.com/idaholab/moose/framework/doc/content/source/constraints/AutomaticMortarGeneration.md)

### Overview

An excellent overview of the conservative mortar constraint implementation in MOOSE for 2D problems is given in
[!cite](osti_1468630). We have verified that the MOOSE mortar implementation satisfies the following *a priori*
error estimates for 2D problems and (see discussion and plots on
[this github issue](https://github.com/idaholab/moose/issues/13080)) and for 3D problems on *hexahedral* meshes:

| Primal FE Type | Lagrange Multiplier (LM) FE Type | Primal L2 Convergence Rate | LM L2 Convergence Rate |
| --- | --- | --- | --- |
| Second order Lagrange | First order Lagrange | 3 | 2.5 |
| Second order Lagrange | Constant monomial | 3 | 1 |
| First order Lagrange | First order Lagrange | 2 | 1.5 |
| First order Lagrange | Constant monomial | 2 | 1.5 |

General meshes in 3D—especially meshes with triangular face elements on the mortar interface—require additional care to ensure convergence.
Triangular elements on the mortar interface typically exhibit the infamous (and well documented) 'locking' phenomenon; resulting in singular systems that require stabilization or other special treatment.

The above *primal* convergence rates were realized on tetrahedral and mixed meshes using a stabilization with `delta = 0.1` for the `EqualValueConstraint`, under the additional assumption that meshes (both generated and unstructured) are re-generated for each experiment.
Uniform refinement of tetrahedral meshes were typically observed to result in *divergence* of the Lagrange multiplier and degradation of primal convergence rates.
Adaptive refinement of meshes with triangular faces on the mortar interface has not been thoroughly studied in MOOSE and should be approached with caution.

Based on these observations the following recommendations are provided for using *3D* mortar in MOOSE:

1. When possible, discretize the secondary side of the mortar interface with QUAD elements (i.e. use HEX elements or carefully oriented PRISM and PYRAMID elements for volume discretization).
2. When TRI elements are present on the mortar interface, verify the wellposedness of the problem and use stabilization if necessary.
3. Avoid uniformly refining mesh, instead regenerate meshes when a refined mesh is needed.


### Parameters

There are four
required parameters the user will always have to supply for a constraint derived
from `MortarConstraint`:

- `primary_boundary`: the boundary name or ID assigned to the primary side of the
  mortar interface
- `secondary_boundary`: the boundary name or ID assigned to the secondary side of the
  mortar interface
- `primary_subdomain`: the subdomain name or ID assigned to the lower-dimesional
  block on the primary side of the mortar interface
- `secondary_boundary`: the subdomain name or ID assigned to the lower-dimensional
  block on the secondary side of the mortar interface

As suggested by the above required parameters, the user must do some mesh work
before they can use a `MortarConstraint` object. The easiest way to prepare
the mesh is to assign boundary IDs to the secondary and primary sides of the
interface when creating the mesh in their 3rd-party meshing software (e.g. Cubit
or Gmsh). If these boundary IDs exist, then the lower dimensional blocks can be
generated automatically using the `LowerDBlockFromSidesetGenerator` mesh generator as
shown in the below input file snippet:

```
[Mesh]
  [./primary]
    type = LowerDBlockFromSidesetGenerator
    sidesets = '2'
    new_block_id = '20'
  [../]
  [./secondary]
    type = LowerDBlockFromSidesetGenerator
    sidesets = '1'
    new_block_id = '10'
  [../]
[]
```

There are also some optional parameters that can be supplied to
`MortarConstraints`. They are:

- `variable`: Corresponds to a Lagrange Multipler variable that lives on the
  lower dimensional block on the secondary face
- `secondary_variable`: Primal variable on the secondary side of the mortar interface
  (lives on the interior elements)
- `primary_variable`: Primal variable on the primary side of the mortar interface
  (lives on the interior elements). Most often `secondary_variable` and
  `primary_variable` will correspond to the same variable
- `compute_lm_residuals`: Whether to compute Lagrange Multiplier residuals. This
  will automatically be set to false if a `variable` parameter is not
  supplied. Other cases where the user may want to set this to false is when a
  different geometric algorithm is used for computing residuals for the LM and
  primal variables. For example, in mechanical contact the Karush-Kuhn-Tucker
  conditions may be enforced at nodes (through perhaps a `NodeFaceConstraint`)
  whereas the contact forces may be applied to the displacement residuals
  through `MortarConstraint`
- `compute_primal_residuals`: Whether to compute residuals for the primal
  variables. Again this may be a useful parameter to use when applying different
  geometric algorithms for computing residuals for LM variables and primal
  variables.
- `periodic`: Whether this constraint is going to be used to enforce a periodic
  condition. This has the effect of changing the normals vector, for mortar
  projection, from outward to inward facing.

At present, either the `secondary_variable` or `primary_variable` parameter must be supplied.

!syntax list /Constraints objects=True actions=False subsystems=False

!syntax list /Constraints objects=False actions=False subsystems=True

!syntax list /Constraints objects=False actions=True subsystems=False
