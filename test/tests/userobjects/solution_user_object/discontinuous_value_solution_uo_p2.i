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

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
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

[UserObjects]
  [./soln]
    type = SolutionUserObject
    mesh = discontinuous_value_solution_uo_p1.e
    system_variables = 'discontinuous_variable continuous_variable'
  [../]
[]

[Postprocessors]

  [./discontinuous_value_left]
    type = TestDiscontinuousValuePP
    variable = discontinuous_variable
    point = '0.25 0.25 0.0'
    solution = soln
  [../]
  [./discontinuous_value_face]
    type = TestDiscontinuousValuePP
    variable = discontinuous_variable
    point = '0.5 0.25 0.0'
    solution = soln
  [../]
  [./discontinuous_value_right]
    type = TestDiscontinuousValuePP
    variable = discontinuous_variable
    point = '0.75 0.25 0.0'
    solution = soln
  [../]

  [./continuous_gradient_left]
    type = TestDiscontinuousValuePP
    variable = continuous_variable
    evaluate_gradient = true
    gradient_component = x
    point = '0.25 0.25 0.0'
    solution = soln
  [../]
  [./continuous_gradient_value_face]
    type = TestDiscontinuousValuePP
    variable = continuous_variable
    evaluate_gradient = true
    gradient_component = x
    point = '0.5 0.25 0.0'
    solution = soln
  [../]
  [./continuous_gradient_right]
    type = TestDiscontinuousValuePP
    variable = continuous_variable
    evaluate_gradient = true
    gradient_component = x
    point = '0.75 0.25 0.0'
    solution = soln
  [../]

[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  file_base = discontinuous_value_solution_uo_p2
  exodus = false
  csv = true
[]
