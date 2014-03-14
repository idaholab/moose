[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 2
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 25
  zmin = 0
  zmax = 0
  elem_type = QUAD4

  uniform_refine = 1
[]

[Variables]
  active = 'c'

  [./c]
    order = THIRD
    family = HERMITE
    [./InitialCondition]
      type = RndBoundingBoxIC
      x1 = 15.0
      x2 = 35.0
      y1 = 0.0
      y2 = 25.0
      mx_invalue = 1.0
      mn_invalue = 0.9
      mx_outvalue = -0.7
      mn_outvalue = -0.8
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
      translation = '0 25 0'
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

  l_max_its = 15
  nl_max_its = 10

  start_time = 0.0
  num_steps = 4
  dt = 2.0

  [./Adaptivity]
    initial_adaptivity = 1
    error_estimator = LaplacianErrorEstimator
    refine_fraction = 0.8
   coarsen_fraction = 0.05
    max_h_level = 2
  [../]
[]

[Outputs]
  file_base = rndbox
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]


