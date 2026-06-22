# SubChannel Mesh Generator Migration

The SubChannel mesh generators now create the subchannel and pin meshes with a single assembly
mesh generator. Inputs that previously chained a subchannel mesh generator into a pin mesh
generator must be updated to use the corresponding assembly mesh generator.

This is a breaking input-file change. The old pin and subchannel mesh generator objects have been
replaced by the assembly mesh generator objects listed below.

| Old generator | New generator |
| :- | :- |
| `SCMQuadSubChannelMeshGenerator` + `SCMQuadPinMeshGenerator` | `SCMQuadAssemblyMeshGenerator` |
| `SCMTriSubChannelMeshGenerator` + `SCMTriPinMeshGenerator` | `SCMTriAssemblyMeshGenerator` |
| `SCMDetailedQuadSubChannelMeshGenerator` + `SCMDetailedQuadPinMeshGenerator` | `SCMDetailedQuadAssemblyMeshGenerator` |
| `SCMDetailedTriSubChannelMeshGenerator` + `SCMDetailedTriPinMeshGenerator` | `SCMDetailedTriAssemblyMeshGenerator` |

The new assembly generators create a mesh with both `subchannel` and `fuel_pins` subdomains. The
old `input = ...` parameter on the pin mesh generator is no longer used because there is no separate
pin mesh generator step.

## Quad Assembly Migration

Old quad inputs used two generators:

!listing! language=moose
[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadSubChannelMeshGenerator
    nx = 3
    ny = 3
    n_cells = 10
    pitch = 0.25
    pin_diameter = 0.125
    side_gap = 0.1
    heated_length = 1
  []

  [fuel_pins]
    type = SCMQuadPinMeshGenerator
    input = sub_channel
    nx = 3
    ny = 3
    n_cells = 10
    pitch = 0.25
    heated_length = 1
  []
[]
!listing-end!

The replacement is one assembly generator:

!listing! language=moose
[QuadSubChannelMesh]
  [assembly]
    type = SCMQuadAssemblyMeshGenerator
    nx = 3
    ny = 3
    n_cells = 10
    pitch = 0.25
    pin_diameter = 0.125
    side_gap = 0.1
    heated_length = 1
  []
[]
!listing-end!

For square-lattice inputs, move the geometry and axial-discretization parameters onto the
`SCMQuadAssemblyMeshGenerator` block. Remove the old pin mesh block and its `input` parameter.

The quad assembly generator supports 1xN and Nx1 subchannel-only meshes. These meshes do not contain
pins. A 2x2 or larger quad assembly contains pins.

## Triangular Assembly Migration

Old triangular inputs used a subchannel generator followed by a pin generator:

!listing! language=moose
[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = 3
    n_cells = 20
    flat_to_flat = 0.056
    heated_length = 0.2
    pitch = 0.012
    pin_diameter = 0.01
    dwire = 0.002
    hwire = 0.0833
  []

  [pins]
    type = SCMTriPinMeshGenerator
    input = subchannel
  []
[]
!listing-end!

The replacement is one assembly generator:

!listing! language=moose
[TriSubChannelMesh]
  [assembly]
    type = SCMTriAssemblyMeshGenerator
    nrings = 3
    n_cells = 20
    flat_to_flat = 0.056
    heated_length = 0.2
    pitch = 0.012
    pin_diameter = 0.01
    dwire = 0.002
    hwire = 0.0833
  []
[]
!listing-end!

For triangular-lattice inputs, move the pin, duct, wire-wrap, and axial-discretization parameters
onto the `SCMTriAssemblyMeshGenerator` block. Remove the old pin mesh block and its `input`
parameter.

Triangular assembly meshes always contain pins. The minimum triangular assembly has `nrings = 2`,
which creates seven pins.

## Detailed Mesh Migration

The detailed visualization mesh generators follow the same pattern. Replace the old chained
subchannel and pin generator blocks with a single detailed assembly generator.

For quad detailed meshes:

!listing! language=moose
[Mesh]
  [assembly]
    type = SCMDetailedQuadAssemblyMeshGenerator
    nx = 3
    ny = 3
    n_cells = 10
    pitch = 0.25
    pin_diameter = 0.125
    side_gap = 0.1
    heated_length = 1
  []
[]
!listing-end!

For triangular detailed meshes:

!listing! language=moose
[Mesh]
  [assembly]
    type = SCMDetailedTriAssemblyMeshGenerator
    nrings = 3
    n_cells = 20
    flat_to_flat = 0.056
    heated_length = 0.2
    pitch = 0.012
    pin_diameter = 0.01
    dwire = 0.002
    hwire = 0.0833
  []
[]
!listing-end!

## Parameter Changes

Most geometry parameters keep the same names and values. The main changes are:

| Old usage | New usage |
| :- | :- |
| `input = subchannel` on the pin mesh generator | Remove it |
| `block_id` for the subchannel mesh block | `subchannel_block_id` |
| `block_id` for the pin mesh block | `pin_block_id` |
| `gap` | `side_gap` |

If a downstream object, material, AuxVariable, or output block restricts by subdomain, update the
restriction to use the new assembly generator subdomain names or IDs. The default generated
subdomain names are `subchannel` for the fluid/subchannel mesh and `fuel_pins` for the pin mesh.

See [SCMQuadAssemblyMeshGenerator.md], [SCMTriAssemblyMeshGenerator.md],
[SCMDetailedQuadAssemblyMeshGenerator.md], and [SCMDetailedTriAssemblyMeshGenerator.md] for the full
parameter lists.
