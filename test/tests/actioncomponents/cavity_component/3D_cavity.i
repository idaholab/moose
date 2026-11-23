# A 3D cylinder component enclosed by a cavity meshed between the cylinder and a box.
# The cylinder axis is along x, and its mesh spans x in [0, 4] within a radius of 1
# of that axis, so the box leaves a gap on every side to mesh the cavity in.
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
    zmin = -3
    zmax = 3
  []
[]

[ActionComponents]
  # The enclosure mesh only serves as an input to the cavity, and the cavity already
  # stitches the cylinder mesh into its own, so there is nothing left to combine
  combine_component_meshes = false
  [cyl]
    type = CylinderComponent
    dimension = 3
    radius = 1
    length = 4
    n_axial = 2
    n_sectors = 2
    # ConcentricCircleMeshGenerator only creates the lateral boundary of the circular
    # face if the face has more than one ring of elements
    n_radial = 2
    block = 'cyl'
  []
  [cavity]
    type = CavityComponent
    dimension = 3
    enclosing_surface_mesh_mg = 'enclosure'
    enclosed_components = 'cyl'
    block = 'cavity'
    # NetGen adds interior points when tetrahedralizing this cavity, which it only
    # allows with a non-zero target volume
    target_element_volume = 4
  []
[]
