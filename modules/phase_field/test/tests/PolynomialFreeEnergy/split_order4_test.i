[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 15
  xmin = 0
  xmax = 125
[]

[GlobalParams]
  polynomial_order = 4
[]

[Variables]
  [./c]
  [../]
  [./w]
  [../]
[]

[ICs]
  [./c_IC]
    type = SmoothCircleIC
    x1 = 0.0
    y1 = 0.0
    radius = 60.0
    invalue = 1.0
    outvalue = 0.1
    int_width = 60.0
    variable = c
  [../]
[]

[Kernels]
  [./c_res]
    type = SplitCHParsed
    variable = c
    kappa_name = kappa
    w = w
    f_name = F
  [../]
  [./w_res]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]
  [./time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
[]

[Materials]
  [./Copper]
    type = PFParamsPolyFreeEnergy
    c = c
    T = 1000 # K
    int_width = 30.0
    length_scale = 1.0e-9
    time_scale = 1.0e-9
    D0 = 3.1e-5 # m^2/s, from Brown1980
    Em = 0.71 # in eV, from Balluffi1978 Table 2
    Ef = 1.28 # in eV, from Balluffi1978 Table 2
    surface_energy = 0.7 # Total guess
  [../]
  [./free_energy]
    type = PolynomialFreeEnergy
    c = c
    derivative_order = 2
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON
  l_max_its = 30
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-8
  start_time = 0.0
  num_steps = 50
  dt = 15
  petsc_options_iname = -pc_type
  petsc_options_value = lu
[]

[Outputs]
  exodus = true
[]
