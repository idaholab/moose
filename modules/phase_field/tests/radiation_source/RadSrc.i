[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 20
  ymax = 20
  elem_type = QUAD4
[]

[GlobalParams]
  polynomial_order = 8
  length_scale = 1e-8
  time_scale = 1.0
[]

[Variables]
  [./cv]
    order = THIRD
    family = HERMITE
  [../]
[]

[ICs]
  [./InitialCondition]
    type = ConstantIC
    value = 1.13e-4
    variable = cv
  [../]
[]

[Kernels]
  [./ie_cv]
    type = TimeDerivative
    variable = cv
  [../]

  [./ch_rad]
    type = CHParsed
    variable = cv
    mob_name = M
    f_name = F
  [../]

  [./ch_int]
    type = CHInterface
    variable = cv
    mob_name = M
    kappa_name = kappa
  [../]

  [./src_cv]
    type = RadiationSource
    variable = cv
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./GenIrrad]
    type = PFParamsPolyFreeEnergy
    block = 0
    c = cv
    T = 1150 #K
    int_width = 11.5 #nm
    Ef = 2.69
    Em = 2.4
    D0 = 2.0e-7
    surface_energy = 2.0
  [../]
  [./free_energy]
    type = PolynomialFreeEnergy
    block = 0
    c = cv
    derivative_order = 3
  [../]
  [./DefectSrc]
    type = RadiationDefectCreation
    block = 0
    Vg = 10.0e-2
    x1 = 0
    y1 = 0
    x2 = 20
    y2 = 20
    eta = cv
    periodic = true
    num_defects = 2
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 20
  l_tol = 1.0e-4
  nl_max_its = 25
  nl_rel_tol = 1e-9

  start_time = 0.0
  num_steps = 4
  dt = 10.0
[]

[Outputs]
  exodus = true
[]
