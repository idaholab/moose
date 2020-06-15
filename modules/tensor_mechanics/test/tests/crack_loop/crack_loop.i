[Mesh]
  file = crack_loop.e
[]

[Problem]
  type = FEProblem
  solve = false
[]

[UserObjects]
  [./crack]
    type = CrackFrontDefinition
    crack_direction_method = CurvedCrackFront
    boundary = 1001
  [../]
[]

[Executioner]
  type = Steady
[]
