[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 1
  ymax = 500
  elem_type = QUAD4
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
  [./myT]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = y
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = c
  [../]
  [./diff2]
    type = Diffusion
    variable = myT
  [../]
[]

[BCs]
  [./left]
    type = FunctionDirichletBC
    variable = c
    boundary = left
    function = x
  [../]
  [./bottom]
    type = FunctionDirichletBC
    variable = myT
    boundary = bottom
    function = y
  [../]
  [./right]
    type = FunctionDirichletBC
    variable = c
    boundary = right
    function = x
  [../]
  [./top]
    type = FunctionDirichletBC
    variable = myT
    boundary = top
    function = y
  [../]
[]

[Materials]
  [./free_energy]
    type = RegularSolutionFreeEnergy
    property_name = F
    c = c
    T = myT
    outputs = out
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
