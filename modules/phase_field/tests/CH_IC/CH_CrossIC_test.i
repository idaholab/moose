[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 7
  ny = 7
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  zmin = 0
  zmax = 50
  elem_type = QUAD4

  uniform_refine = 1
[]

[Variables]
  active = 'c'

  [./c]
    order = THIRD
    family = HERMITE
    [./InitialCondition]
      type = CrossIC
      x1 = 0.0
      x2 = 50.0
      y1 = 0.0
      y2 = 50.0
    [../]
  [../]
[]

[Kernels]
  active = 'ie_c CHSolid CHInterface'

  [./ie_c]
    type = TimeDerivative
    variable = c
  [../]

  [./CHSolid]
    type = CHMath
    variable = c
    mob_name = M
  [../]

  [./CHInterface]
    type = CHInterface
    variable = c
    kappa_name = kappa_c
    mob_name = M
    grad_mob_name = grad_M
  [../]
[]

[BCs]
active = 'Periodic'
  [./Periodic]
    [./left_right]
      primary = 0
      secondary = 2
      translation = '0 50 0'
    [../]

    [./top_bottom]
      primary = 1
      secondary = 3
      translation = '-50 0 0'
    [../]
  [../]
[]

[Materials]

  [./constant]
    type = PFMobility
    block = 0
    mob = 1.0
    kappa = 2.0
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 101'

  l_max_its = 10
  l_tol = 1.0e-4

  nl_max_its = 10
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-13

  start_time = 0.0
  num_steps = 2
  dt = 1.0

  [./Adaptivity]
    initial_adaptivity = 1
    error_estimator = LaplacianErrorEstimator
    refine_fraction = 0.7
   coarsen_fraction = 0.05
    max_h_level = 2
  [../]
[]

[Outputs]
  file_base = cross
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]


