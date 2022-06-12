# a wrapper mesh for coupling to subchannel

# sqrt(3) / 2 is by how much flat to flat is smaller than corer to corner
f = ${fparse sqrt(3) / 2}

# units are meters
height = 2
duct_inside = 0.185
wrapper_thickness = 0.002
duct_outside = ${fparse duct_inside + 2 * wrapper_thickness}

# number of radial elements in the wrapper
n_radial = 4

# number of azimuthal elements per side
n_az = 4

# number of axial elements
n_ax = 10

[Mesh]
  [2d_fuel_element]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${n_az} ${n_az} ${n_az} ${n_az} ${n_az} ${n_az}'
    background_intervals = 1
    background_block_ids = '1'
    # note that polygon_size is "like radius"
    polygon_size = ${fparse duct_outside / 2}
    duct_sizes = '${fparse duct_inside / 2 / f}'
    duct_intervals = '${n_radial}'
    duct_block_ids = '2'
    interface_boundary_names = 'inside'
    external_boundary_name = 'outside'
  []

  [extrude]
    type = FancyExtruderGenerator
    direction = '0 0 1'
    input = 2d_fuel_element
    heights = '${height}'
    num_layers = '${n_ax}'
  []

  [remove]
    type = BlockDeletionGenerator
    block = 1
    input = extrude
  []

  [rename]
    type = RenameBlockGenerator
    input = remove
    old_block = '2'
    new_block = 'wrapper'
  []

  [rotate]
    type = TransformGenerator
    input = rename
    transform = ROTATE
    vector_value = '30 0 0'
  []
[]
