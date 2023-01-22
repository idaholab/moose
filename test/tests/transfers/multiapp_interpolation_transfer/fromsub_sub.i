[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = .21
  xmax = .79
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./disp_x]
    initial_condition = -0.4
  [../]
  [./disp_y]
  [../]
  [./elemental]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./x_func]
    type = ParsedFunction
    expression = x
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./x_func_aux]
    type = FunctionAux
    variable = elemental
    function = x_func
    execute_on = initial
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
