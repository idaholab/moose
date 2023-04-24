[Mesh]
  [transf]
    type = TransfiniteMeshGenerator
    corners = '-0.25 0.25 0.0
              0.25 0.25 0.0
              0.707106781186547 0.707106781186547 0
              -0.707106781186547 0.707106781186547 0'
    nx = 20
    ny = 6
    bottom_type = LINE
    top_type = CIRCARC
    right_type = LINE
    left_type = LINE
    top_parameter = '0.0 ; 1.0 ; 0.0'
  []
[]

[Outputs]
  exodus = true
[]
