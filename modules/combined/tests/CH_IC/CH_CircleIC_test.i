[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
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
      type = SmoothCircleIC
      x1 = 25.0
      y1 = 25.0
      radius = 6.0
      invalue = 1.0
      outvalue = -0.8
      int_width = 0.0 #4.0
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
    kappa = 1.0
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 101'

  l_max_its = 20
  l_tol = 1.0e-5

  nl_max_its = 40
  nl_rel_tol = 5.0e-14

  start_time = 0.0
  num_steps = 2
  dt = 2.0

  [./Adaptivity]
    initial_adaptivity = 1
    error_estimator = LaplacianErrorEstimator
    refine_fraction = 0.8
    coarsen_fraction = 0.05
    max_h_level = 2
  [../]
[]

[Output]
  file_base = circle
  output_initial = true
  interval = 1
  exodus = false
  perf_log = true

  [./OverSampling]
    exodus = true
    refinements = 3
    output_initial = true
    interval = 1
  [../]

[]


