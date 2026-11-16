# Two disconnected cylinder components enclosed by a single cavity. Each cylinder
# spans x in [0, 4] within a radius of 1 of its own axis, and they are offset in z,
# so the box encloses both with a gap around each to mesh the cavity in.
[Mesh]
  [enclosure]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = -3
    xmax = 5
    ymin = -3
    ymax = 3
    zmin = -5
    zmax = 5
  []
[]

[ActionComponents]
  # The enclosure mesh only serves as an input to the cavity, and the cavity already
  # stitches the cylinder meshes into its own, so there is nothing left to combine
  combine_component_meshes = false
  [cyl_1]
    type = CylinderComponent
    dimension = 3
    radius = 1
    length = 4
    n_axial = 2
    n_sectors = 2
    # ConcentricCircleMeshGenerator only creates the lateral boundary of the circular
    # face if the face has more than one ring of elements
    n_radial = 2
    position = '0 0 2.5'
    block = 'cyl_1'
  []
  [cyl_2]
    type = CylinderComponent
    dimension = 3
    radius = 1
    length = 4
    n_axial = 2
    n_sectors = 2
    n_radial = 2
    position = '0 0 -2.5'
    block = 'cyl_2'
  []
  [cavity]
    type = CavityComponent
    dimension = 3
    enclosing_surface_mesh_mg = 'enclosure'
    enclosed_components = 'cyl_1 cyl_2'
    block = 'cavity'
    # NetGen adds interior points when tetrahedralizing this cavity, which it only
    # allows with a non-zero target volume
    target_element_volume = 4
  []
[]
