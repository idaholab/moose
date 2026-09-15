[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./discontinuous_variable]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./continuous_variable]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./discontinuous_function]
    type = ParsedFunction
    expression = 'if(x<0.5,3,5)'
  [../]
  [./continuous_function]
    type = ParsedFunction
    expression = 'if(x<0.5,x,2*x-0.5)'
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[ICs]
  [./discontinuous_variable]
    type = FunctionIC
    variable = discontinuous_variable
    function = discontinuous_function
  [../]
  [./continuous_variable]
    type = FunctionIC
    variable = continuous_variable
    function = continuous_function
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./one]
    type = DirichletBC
    variable = u
    boundary = 'right top bottom'
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  file_base = discontinuous_value_solution_uo_p1
  exodus = true
[]
