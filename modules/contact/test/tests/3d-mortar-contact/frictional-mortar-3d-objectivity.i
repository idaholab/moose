!include frictional-mortar-3d-penalty.i

[Mesh]
  [rotate_all]
    type = TransformGenerator
    input = primary
    transform = ROTATE_WITH_MATRIX
    rotation_matrix = '1 0 0 0 1 0 0 0 1'
  []
[]

[Problem]
  type = FEProblem
[]

[Postprocessors]
  [normal_sum]
    type = NodalSum
    variable = penalty_normal_pressure
    boundary = top_bottom
  []
[]
