[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  parallel_type = replicated
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
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

[MultiApps]
  [sub_app]
    type = FullSolveMultiApp
    input_files = "checkpoint_child_ss.i"
    positions = '0 0 0'
  []
[]

[Executioner]
  type = Steady
  fixed_point_min_its = 3
  fixed_point_max_its = 3
  l_abs_tol = 1e-12
  nl_abs_tol = 1e-12
  line_search = none
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]
