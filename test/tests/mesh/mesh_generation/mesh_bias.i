[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  xmin = -0.5
  xmax = 0.5
  ymin = -1
  ymax = 1
  zmin = -1.5
  zmax = 1.5
  elem_type = HEX8
  # Bias parameters tested in this test
  bias_x = 0.75
  bias_y = 1.25
  bias_z = 1.0
[]

[Outputs]
  exodus = true
[]
