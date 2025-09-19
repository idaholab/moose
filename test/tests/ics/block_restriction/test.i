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
  second_order = true
[]

[Variables]
  [c1]
  []
[]

[AuxVariables]
  [c2]
    order = second
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
  [c2]
    type = SolutionIC
    # aux variables are not stored in solutionUO
    from_variable = 'c1'
    solution_uo = 2phase
    variable = c2
    block = 0
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
