[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 25
  ny = 25
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  zmin = 0
  zmax = 50
  elem_type = QUAD4
[]

[Variables]
  active = 'c'

  [./c]
    order = THIRD
    family = HERMITE
  [../]
[]

[ICs]
  [./c]
    variable = c
    type = RndSmoothCircleIC
    x1 = 25.0
    y1 = 25.0
    radius = 6.0
    invalue = 1.0
    variation_invalue = 0.0
    outvalue = -0.8;
    variation_outvalue = 0.2
    int_width = 5
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
    kappa = 1.0
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  #petsc_options = '-snes_ksp_ew'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 101'

  l_max_its = 10
  l_tol = 1.0e-3

  nl_max_its = 10

  start_time = 0.0
  num_steps = 1
  dt = 1.0
[]

[Outputs]
  file_base = rnd_circle
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]


