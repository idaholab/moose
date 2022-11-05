[Mesh]
  type = GeneratedMesh
  dim = 1
  xmax = 1000
  nx = 25
[]

[GlobalParams]
  polynomial_order = 8
[]

[Variables]
  [./c]
    order = THIRD
    family = HERMITE
  [../]
[]

[ICs]
  [./c_IC]
    type = SmoothCircleIC
    x1 = 175.0
    y1 = 0.0
    radius = 100
    invalue = 1.0
    outvalue = 0.01
    int_width = 100.0
    variable = c
  [../]
[]

[AuxVariables]
  [./T]
  [../]
[]

[Kernels]
  [./c_int]
    type = CHInterface
    variable = c
    kappa_name = kappa
    mob_name = M
  [../]
  [./c_bulk]
    type = CahnHilliard
    variable = c
    mob_name = M
    f_name = F
  [../]
  [./c_soret]
    type = SoretDiffusion
    variable = c
    T = T
    diff_name = D
    Q_name = Qstar
  [../]
  [./c_dot]
    type = TimeDerivative
    variable = c
  [../]
[]

[AuxKernels]
  [./Temp]
    type = FunctionAux
    variable = T
    function = 1000.0+0.025*x
  [../]
[]

[Materials]
  [./Copper]
    type = PFParamsPolyFreeEnergy
    block = 0
    c = c
    T = T # K
    int_width = 80.0
    length_scale = 1.0e-9
    time_scale = 1.0e-6
    D0 = 3.1e-5 # m^2/s, from Brown1980
    Em = 0.71 # in eV, from Balluffi1978 Table 2
    Ef = 1.28 # in eV, from Balluffi1978 Table 2
    surface_energy = 0.708 # Total guess
  [../]
  [./free_energy]
    type = PolynomialFreeEnergy
    block = 0
    c = c
    outputs = exodus
    derivative_order = 3
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  solve_type = 'NEWTON'

  l_max_its = 10
  l_tol = 1.0e-4
  nl_max_its = 25
  nl_rel_tol = 1.0e-9

  start_time = 0.0
  num_steps = 60
  dt = 1
[]

[Outputs]
  exodus = true
[]
