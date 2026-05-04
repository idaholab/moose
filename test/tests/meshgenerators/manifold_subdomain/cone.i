expected = '2975'

!include check.i

[Mesh]
  [disk]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '0.9'
    rings = '10'
    has_outer_square = false
    preserve_volumes = false
  []
  [cone]
    type = ParsedNodeTransformGenerator
    input = disk
    z_function = '0.9 - sqrt(x * x + y * y)'
  []
  [surface_quads]
    type = StitchMeshGenerator
    inputs = 'cone disk'
    stitch_boundaries_pairs = 'outer outer'
    clear_stitched_boundary_ids = true
  []

  [background]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 25
    ny = 25
    nz = 25
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    zmax = 1
  []

  [surface]
    type = ElementsToSimplicesConverter
    input = surface_quads
  []
  [tag]
    type = ManifoldSubdomainGenerator
    input = background
    manifold = surface
    block_id = 1
  []
  [remove]
    type = BlockDeletionGenerator
    input = tag
    block = '0'
  []
[]
