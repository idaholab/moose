[Mesh]
  [transf]
    type = TransfiniteMeshGenerator
    corners = '-1.0 -1.0 0.0
              10.0 -1.0 0.0
              11.0 5.0 0
              4.0 4.0 0'
    nx = 10
    ny = 7
    left_type = DISCRETE
    right_type = LINE
    top_type = CIRCARC
    bottom_type = LINE
    left_parameter = '-1.0 -1.0 0
                      -0.7 -0.7 0.0
                      -0.5 -0.5 0.0
                      0.0 0.0 0.0
                     0.75 0.75 0.0
                     2.5 2.5 0
                     4.0 4.0 0.0'
    top_parameter ='-1.17'
  []
[]

[Outputs]
  exodus = true
  xda = true
[]
