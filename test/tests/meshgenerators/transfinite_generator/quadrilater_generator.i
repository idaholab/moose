[Mesh]
  [transf]
    type = TransfiniteMeshGenerator
    corners = '-1.0 -1.0 0.0
              10.0 -1.0 0.0
              11.0 5.0 0
              4.0 4.0 0'
    nx = 5
    ny = 8
    bottom_type = LINE
    top_type = LINE
    right_type = LINE
    left_type = LINE
  []
[]

[Outputs]
  exodus = true
  xda = true
[]
