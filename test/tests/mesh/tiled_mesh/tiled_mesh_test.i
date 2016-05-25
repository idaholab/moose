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

  # You can only run this test with ReplicatedMesh because the underlying
  # algorithm, stitch_meshes(), only works with ReplicatedMesh.
  parallel_type = replicated
[]
