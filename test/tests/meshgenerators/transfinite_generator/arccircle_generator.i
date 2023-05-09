[Mesh]
  [transf]
    type = TransfiniteMeshGenerator
    corners = '-1.0 -1.0 0.0
              10.0 -1.0 0.0
              11.0 5.0 0
              4.0 4.0 0'
    nx = 10
    ny = 6
    top_type = LINE
    bottom_type = CIRCARC
    right_type = LINE
    left_type = LINE
    bottom_parameter = '-1.23'
  []
[]

[Outputs]
  exodus = true
[]
