# A 2D cylinder component enclosed by a cavity. The enclosing line mesh is not
# written by hand: it is extracted from the perimeter of a quadrilateral mesh
# built in the [Mesh] block.
# The cylinder mesh spans x in [0, 4] and y in [-1, 1], so the enclosure leaves a
# gap on every side to mesh the cavity in.
[Mesh]
  [box]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 3
    xmin = -2
    xmax = 6
    ymin = -3
    ymax = 3
  []
  [perimeter_block]
    type = LowerDBlockFromSidesetGenerator
    input = 'box'
    sidesets = 'left right top bottom'
    new_block_name = 'perimeter'
  []
  [enclosure]
    type = BlockToMeshConverterGenerator
    input = 'perimeter_block'
    target_blocks = 'perimeter'
  []
  # The [Mesh] generators only feed the cavity, they do not form a mesh of their own.
  # The cavity stitches the cylinder into its own mesh, so it is the final mesh
  final_generator = 'cavity_triangulation'
[]

[ActionComponents]
  [cyl]
    type = CylinderComponent
    dimension = 2
    radius = 1
    length = 4
    n_axial = 2
    n_radial = 2
    block = 'cyl'
  []
  [cavity]
    type = CavityComponent
    dimension = 2
    enclosing_surface_mesh_mg = 'enclosure'
    enclosed_components = 'cyl'
    block = 'cavity'
  []
[]
