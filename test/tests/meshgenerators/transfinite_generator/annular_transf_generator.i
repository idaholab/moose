[Mesh]
  [transf]
    type = TransfiniteMeshGenerator
    corners = '1.0 0.0 0.0
              5.0 0.0 0.0
              0 5.0 0
              0 1.0 0'
    nx = 6
    ny = 8
    top = LINE
    bottom = LINE
    right = CIRCARC
    left = CIRCARC
    bias_x=1.3
    bias_y=1.0
    right_parameter = ' 1.464466094067262'
    left_parameter = '-0.292893218813452'
  []
[]

[Outputs]
  exodus = true
[]
