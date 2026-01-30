# Simplified mesh for DOME shield, not including cavity or reactor.
#
# Other simplifications:
#   - All concrete share the same block, despite there being different grades
#   - Water tank walls are neglected
#
# Run as:   <executable> -i generate_mesh.i --mesh-only shield_mesh.e

dx_door = ${units 10 ft -> m}
dz_floor = ${units 2.75 ft -> m}
dz_ceiling = ${units 7.25 ft -> m}
t_wall = ${units 4.375 ft -> m} # back wall and side walls
t_water = ${units 2.625 ft -> m}
dx_cavity = ${units 30 ft -> m}
dy_cavity = ${units 15 ft -> m}
dz_cavity = ${units 13 ft -> m}

x_cavity_begin = ${dx_door}
x_cavity_end = ${fparse x_cavity_begin + dx_cavity}

concrete_blockid = 100
water_blockid = 101
cavity_blockid = 102 # note the cavity is deleted

# Outer dimensions:  29'x47'x23'; volume = 31,349 ft^3 ~ 887.70 m^3
# Cavity dimensions: 30'x15'x13'; volume =  5,850 ft^3 ~ 165.65 m^3
# Total volume ~ 722.05 m^3 (Verify "Global mesh volume" in console matches this when running this input)

[Mesh]
  [cartesian_mesh]
    type = CartesianMeshGenerator
    dim = 3
    dx = '${dx_door} ${dx_cavity} ${t_water} ${t_wall}'
    ix =  '1 4 1 1'
    dy = '${t_wall} ${t_water} ${dy_cavity} ${t_water} ${t_wall}'
    iy =  '1 1 4 1 1'
    dz = '${dz_floor} ${dz_cavity} ${dz_ceiling}'
    iz =  '1 4 2'
  []
  [concrete_blockid]
    type = RenameBlockGenerator
    input = cartesian_mesh
    old_block = 0
    new_block = ${concrete_blockid}
  []

  # shift y to center
  [y_shift]
    type = TransformGenerator
    input = concrete_blockid
    transform = TRANSLATE
    vector_value = '0 -${fparse t_wall + t_water + 0.5*dy_cavity} 0'
  []

  [water_block]
    type = ParsedSubdomainMeshGenerator
    input = y_shift
    combinatorial_geometry = 'x > ${x_cavity_begin} & x < ${fparse x_cavity_end + t_water} & y > -${fparse 0.5*dy_cavity + t_water} & y < ${fparse 0.5*dy_cavity + t_water} & z > ${dz_floor} & z < ${fparse dz_floor + dz_cavity}'
    block_id = ${water_blockid}
    block_name = water
  []
  [cavity_block]
    type = ParsedSubdomainMeshGenerator
    input = water_block
    combinatorial_geometry = 'x > ${x_cavity_begin} & x < ${x_cavity_end} & y > -${fparse 0.5*dy_cavity} & y < ${fparse 0.5*dy_cavity} & z > ${dz_floor} & z < ${fparse dz_floor + dz_cavity}'
    block_id = ${cavity_blockid}
    block_name = cavity
  []

  [delete_cavity]
    type = BlockDeletionGenerator
    input = cavity_block
    block = cavity
    new_boundary = inner_wall
  []

  [rename_concrete]
    type = RenameBlockGenerator
    input = delete_cavity
    old_block = ${concrete_blockid}
    new_block = concrete
  []
  [rename_boundaries]
    type = RenameBoundaryGenerator
    input = rename_concrete
    old_boundary = 'left right bottom top back front'
    new_boundary = 'walls_outer walls_outer walls_outer walls_outer floor_outer walls_outer'
  []
[]
