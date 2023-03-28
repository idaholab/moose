[Mesh]
  [transf]
    type = TransfiniteMeshGenerator
    corners = '-1.0 -0.5 0.0
                1.0 -0.5 0.0
                1.0 0.5 0.0
               -1.0 0.5 0.0'
    nx = 10
    ny = 6
    bottom_type = PARSED
    top_type = PARSED
    right_type = LINE
    left_type = LINE
    top_parameter = '2*r-1 ; 3/8 - cos(pi*(2*r-1))/8'
    bottom_parameter = '2*r-1 ; cos(pi*(2*r-1))/8 - 3/8'
  []
[]

[Outputs]
  exodus = true
[]
