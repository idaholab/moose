[Mesh]
  [transf]
    type = TransfiniteMeshGenerator
    corners = '-1.0 -0.5 0.0
                1.0 -0.5 0.0
                1.0 0.5 0.0
               -1.0 0.5 0.0'
    nx = 8 
    ny = 6 
    bottom = PARSED
    top = PARSED
    right = LINE
    left = LINE
    top_parameter = '2*x-1 && 3/8 - cos(pi*(2*x-1))/8'
    bottom_parameter = '2*x-1 && cos(pi*(2*x-1))/8 - 3/8'
  []
[]

[Outputs]
  exodus = true
[]
