[Mesh]
  [./ccmg]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '0.2546 0.3368 0.3600 0.3818 0.3923 0.4025 0.4110 0.4750'
    rings = '5 3 2 1 1 1 1 3 5'
    has_outer_square = on
    pitch = 1.42063
    #portion = left_half
    preserve_volumes = off
    smoothing_max_it = 3
  []
[]

[Outputs]
  exodus = true
[]
