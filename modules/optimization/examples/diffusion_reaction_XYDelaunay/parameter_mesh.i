[Mesh]
  [outer_bdy]
    type = PolyLineMeshGenerator
    points = '0.0 0.0 0.0
              0.5 0.0 0.0
              1.0 0.0 0.0
              1.0 0.5 0.0
              1.0 0.75 0.0
              1.0 0.875 0.0
              1.0 1.0 0.0
              0.875 1.0 0.0
              0.75 1.0 0.0
              0.5 1.0 0.0
              0.0 1.0 0.0
              0.0 0.5 0.0'
    loop = true
  []
  [triangulate]
    type = XYDelaunayGenerator
    boundary = 'outer_bdy'
    interior_points = '0.5 0.5 0
    0.5 0.75 0
    0.75 0.5 0
    0.75 0.75 0
    0.75 0.875 0
    0.875 0.75 0
    0.875 0.875 0'
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  # To allow for exodus outputting.
  [dummy]
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  execute_on = TIMESTEP_END
[]

