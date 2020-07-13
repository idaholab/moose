starting_point = .5

[GlobalParams]
  displacements = 'disp_x disp_y'
  diffusivity = 1
[]

[Mesh]
  file = square-blocks-no-offset.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[ICs]
  [./disp_y]
    block = 2
    variable = disp_y
    value = ${starting_point}
    type = ConstantIC
  [../]
[]

[Kernels]
  [./disp_x]
    type = MatDiffusion
    variable = disp_x
  [../]
  [./disp_y]
    type = MatDiffusion
    variable = disp_y
  [../]
[]


[Constraints]
  [./disp_x]
    type = RANFSNormalMechanicalContact
    secondary = 10
    primary = 20
    variable = disp_x
    primary_variable = disp_x
    component = x
  [../]
  [./disp_y]
    type = RANFSNormalMechanicalContact
    secondary = 10
    primary = 20
    variable = disp_y
    primary_variable = disp_y
    component = y
  [../]
[]

[BCs]
  [./botx]
    type = DirichletBC
    variable = disp_x
    preset = false
    boundary = 40
    value = 0.0
  [../]
  [./topx]
    type = DirichletBC
    variable = disp_x
    preset = false
    boundary = 30
    value = 0.0
  [../]
  [./boty]
    type = DirichletBC
    variable = disp_y
    preset = false
    boundary = 40
    value = 0.0
  [../]
  [./topy]
    type = FunctionDirichletBC
    variable = disp_y
    preset = false
    boundary = 30
    function = '${starting_point} - t'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
  dtmin = 1
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -ksp_monitor_true_residual -snes_view'
  petsc_options_iname = '-mat_mffd_err -pc_type -pc_hypre_type'
  petsc_options_value = '1e-5          hypre    boomeramg'
  l_max_its = 30
  nl_max_its = 20
  line_search = 'project'
  snesmf_reuse_base = false
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  [exo]
    type = Exodus
    execute_on = 'nonlinear'
  []
  print_linear_residuals = false
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [./num_nl]
    type = NumNonlinearIterations
  [../]
  [./cumulative]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  [../]
[]
