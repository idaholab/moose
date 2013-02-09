#[Mesh]
#  type = GeneratedMesh
#  dim = 3
#  nx = 4
#  ny = 4
#  nz = 4
#
#  xmax = 10
#  ymax = 10
#  zmax = 10
#  elem_type = HEX8
#[]

[Mesh]
  type = TiledMesh
  file = cube.e

  x_width = 10
  y_width = 10
  z_width = 10

  left_boundary = left
  right_boundary = right
  top_boundary = top
  bottom_boundary = bottom
  front_boundary = front
  back_boundary = back

  x_tiles = 2
  y_tiles = 2
  z_tiles = 2
[]