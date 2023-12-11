[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [rg]
    type = RevolveGenerator
    input = gmg
    axis_point = '0.0 0.0 0.0'
    axis_direction = '0.0 1.0 0.0'
    nums_azimuthal_intervals = 6
  []
[]
