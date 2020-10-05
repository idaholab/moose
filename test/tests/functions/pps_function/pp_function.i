[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

[Variables]
  active = 'u'
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./function_force]
    function = pp_func
    variable = u
    type = BodyForce
  [../]
[]

[BCs]
  active = 'left right'
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
  exodus = true
[]

[Functions]
  [./pp_func]
    pp = right_value
    type = PostprocessorFunction
  [../]
[]

[Postprocessors]
  [./right_value]
    variable = u
    execute_on = linear
    boundary = 1
    type = SideAverageValue
  [../]
[]
