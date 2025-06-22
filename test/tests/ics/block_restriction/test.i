[Mesh]
  [main]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 9
    xmax = 4
    ymax = 3
  []
  [top_block]
    type = ParsedSubdomainMeshGenerator
    input = main
    block_id = 1
    combinatorial_geometry = 'y > 1'
  []
[]

[Variables]
  [c1]
  []
[]

[ICs]
  [c1]
    type = SolutionIC
    from_variable = 'c1'
    solution_uo = 2phase
    variable = c1
    block = 0
  []
  [top_c1]
    type = ConstantIC
    value = 0.1
    variable = c1
    block = 1
  []
[]

[UserObjects]
  [2phase]
    type = SolutionUserObject
    mesh = 'start_out.e'
    system_variables = 'c1'
    timestep = LATEST
  []
[]


[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
