[Mesh]
  [transf]
    type = TransfiniteMeshGenerator
    corners = '-1.0 -1.0 0.0
              10.0 -1.0 0.0
              11.0 5.0 0
              4.0 4.0 0'
    nx = 12 
    ny = 6 
    top = LINE
    bottom = CIRCARC
    right = LINE
    left = LINE
    bottom_parameter = '-1.23'              
  []
[]

[Outputs]
  exodus = true
[]
