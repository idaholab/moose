starting_point = 2e-1
offset = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
  diffusivity = 1e0
  ping_pong_protection = true
[]

[Mesh]
  file = long-bottom-block-no-lower-d.e
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
    value = ${fparse starting_point + offset}
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

[Contact]
  [./top_bottom]
    secondary = 10
    primary = 20

    model = frictionless
    formulation = ranfs
  [../]
[]

[BCs]
  [./botx]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  [../]
  [./boty]
    type = DirichletBC
    variable = disp_y
    boundary = 40
    value = 0.0
  [../]
  [./topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 30
    function = '${starting_point} * cos(2 * pi / 40 * t) + ${offset}'
  [../]
  [./leftx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 50
    function = '1e-2 * t'
  [../]
[]

[Executioner]
  type = Transient
  end_time = 200
  dt = 5
  dtmin = 2.5
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -pc_hypre_type -mat_mffd_err'
  petsc_options_value = 'hypre    boomeramg      1e-5'
  l_max_its = 30
  nl_max_its = 20
  line_search = 'none'
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  [exo]
    type = Exodus
  []
  checkpoint = true
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
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
