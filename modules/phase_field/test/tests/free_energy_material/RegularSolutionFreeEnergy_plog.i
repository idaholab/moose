[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 50
  xmax = 1
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = c
  [../]
[]

[BCs]
  [./left]
    type = FunctionDirichletBC
    variable = c
    boundary = left
    function = x
  [../]
  [./right]
    type = FunctionDirichletBC
    variable = c
    boundary = right
    function = x
  [../]
[]

[Materials]
  [./free_energy]
    type = RegularSolutionFreeEnergy
    property_name = F
    c = c
    outputs = out
    log_tol = 0.2
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  l_max_its = 1
  nl_max_its = 1
  nl_abs_tol = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  [./out]
    type = Exodus
    execute_on = timestep_end
  [../]
[]
