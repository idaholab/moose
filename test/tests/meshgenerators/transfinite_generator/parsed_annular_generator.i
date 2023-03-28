[Mesh]
  [transf]
    type = TransfiniteMeshGenerator
    corners = '1.0 0.0 0.0
              5.0 0.0 0.0
              0 5.0 0
              0 1.0 0'
    nx = 8
    ny = 6
    left_type = PARSED
    right_type = PARSED
    top_type = LINE
    bottom_type = LINE
    left_parameter = 'cos(r*pi/2) ; sin(r*pi/2)'
    right_parameter = '5*cos(r*pi/2) ; 5*sin(r*pi/2)'
  []
[]

[Outputs]
  exodus = true
[]
