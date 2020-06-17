# Constraints System

## MortarConstraints

### Overview

An excellent overview of the conservative mortar constraint implementation in MOOSE is given in
[!cite](osti_1468630). We have verified that the MOOSE mortar implementation satisfies *a priori*
error estimates (see discussion and plots on
[this github issue](https://github.com/idaholab/moose/issues/13080)):

| Primal FE Type | Lagrange Multiplier (LM) FE Type | Primal L2 Convergence Rate | LM L2 Convergence Rate |
| --- | --- | --- | --- |
| Second order Lagrange | First order Lagrange | 3 | 2.5 |
| Second order Lagrange | Constant monomial | 3 | 1 |
| First order Lagrange | First order Lagrange | 2 | 1.5 |
| First order Lagrange | Constant monomial | 2 | 1.5 |

### Parameters

There are four
required parameters the user will always have to supply for a constraint derived
from `MortarConstraint`:

- `master_boundary`: the boundary name or ID assigned to the master side of the
  mortar interface
- `secondary_boundary`: the boundary name or ID assigned to the secondary side of the
  mortar interface
- `master_subdomain`: the subdomain name or ID assigned to the lower-dimesional
  block on the master side of the mortar interface
- `secondary_boundary`: the subdomain name or ID assigned to the lower-dimensional
  block on the secondary side of the mortar interface

As suggested by the above required parameters, the user must do some mesh work
before they can use a `MortarConstraint` object. The easiest way to prepare
the mesh is to assign boundary IDs to the secondary and master sides of the
interface when creating the mesh in their 3rd-party meshing software (e.g. Cubit
or Gmsh). If these boundary IDs exist, then the lower dimensional blocks can be
generated automatically using the `LowerDBlockFromSideset` mesh modifiers as
shown in the below input file snippet:

```
[MeshModifiers]
  [./master]
    type = LowerDBlockFromSideset
    sidesets = '2'
    new_block_id = '20'
  [../]
  [./secondary]
    type = LowerDBlockFromSideset
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
- `master_variable`: Primal variable on the master side of the mortar interface
  (lives on the interior elements). Most often `secondary_variable` and
  `master_variable` will correspond to the same variable
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

At present, either the `secondary_variable` or `master_variable` parameter must be supplied.

### Limitations

Unfortunately the mortar system does not currently work in three dimensions. It
is on the to-do list, but it will require a significant amount of work to get
all the projections correct.

!syntax list /Constraints objects=True actions=False subsystems=False

!syntax list /Constraints objects=False actions=False subsystems=True

!syntax list /Constraints objects=False actions=True subsystems=False
