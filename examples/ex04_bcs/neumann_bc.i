[Mesh]
  file = square.e
  uniform_refine = 4
[]

[Variables]
  [./convected]
    order = FIRST
    family = LAGRANGE
  [../]

  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff_convected]
    type = Diffusion
    variable = convected
  [../]

  [./conv]
    type = ExampleConvection
    variable = convected
    some_variable = diffused
  [../]

  [./diff_diffused]
    type = Diffusion
    variable = diffused
  [../]
[]

[BCs]
  active = 'left_convected right_convected_neumann left_diffused right_diffused'

  [./left_convected]
    type = DirichletBC
    variable = convected
    boundary = 'left'
    value = 0
  [../]

  # Note: This BC is not active in this input file
  [./right_convected_dirichlet]
    type = CoupledDirichletBC
    variable = convected
    boundary = 'right'
    alpha = 2

    some_var = diffused
  [../]

  [./right_convected_neumann]
    type = CoupledNeumannBC
    variable = convected
    boundary = 'right'
    alpha = 2

    some_var = diffused
  [../]

  [./left_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 'left'
    value = 0
  [../]

  [./right_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 'right'
    value = 1
  [../]

[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
