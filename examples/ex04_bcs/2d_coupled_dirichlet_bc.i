[Mesh]
  file = square.e
  uniform_refine = 4
[]

[Variables]
  active = 'convected diffused'

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
  active = 'diff_convected conv diff_diffused'

  [./diff_convected]
    type = Diffusion
    variable = convected
  [../]

  [./conv]
    type = Convection
    variable = convected
    some_variable = diffused
  [../]

  [./diff_diffused]
    type = Diffusion
    variable = diffused
  [../]
[]

[BCs]
  active = 'left_convected right_convected_dirichlet left_diffused right_diffused'

  [./left_convected]
    type = DirichletBC
    variable = convected
    boundary = '1'
    value = 0
  [../]

  [./right_convected_dirichlet]
    type = CoupledDirichletBC
    variable = convected
    boundary = '2'
    value = 2

    some_var = diffused
  [../]

  # Note: This BC is not active in this input file
  [./right_convected_neumann]
    type = CoupledNeumannBC
    variable = convected
    boundary = '2'
    value = 2
    
    some_var = diffused
  [../]

  [./left_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '1'
    value = 0
  [../]

  [./right_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '2'
    value = 1
  [../]

[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out_coupled_dirichlet
  interval = 1
  exodus = true
  perf_log = true
[]
   
    
