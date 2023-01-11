[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 60
  xmax = 500
  elem_type = EDGE
[]

[GlobalParams]
  polynomial_order = 8
[]

[Variables]
  [./c]
  [../]
  [./w]
    scaling = 1.0e2
  [../]
  [./T]
    initial_condition = 1000.0
    scaling = 1.0e5
  [../]
[]

[ICs]
  [./c_IC]
    type = SmoothCircleIC
    x1 = 125.0
    y1 = 0.0
    radius = 60.0
    invalue = 1.0
    outvalue = 0.1
    int_width = 100.0
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
  [./w_res_soret]
    type = SoretDiffusion
    variable = w
    c = c
    T = T
    diff_name = D
    Q_name = Qstar
  [../]
  [./time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
  [./HtCond]
    type = MatDiffusion
    variable = T
    diffusivity = thermal_conductivity
  [../]
[]

[BCs]
  [./Left_T]
    type = DirichletBC
    variable = T
    boundary = left
    value = 1000.0
  [../]

  [./Right_T]
    type = DirichletBC
    variable = T
    boundary = right
    value = 1015.0
  [../]
[]

[Materials]
  [./Copper]
    type = PFParamsPolyFreeEnergy
    block = 0
    c = c
    T = T # K
    int_width = 60.0
    length_scale = 1.0e-9
    time_scale = 1.0e-9
    D0 = 3.1e-5 # m^2/s, from Brown1980
    Em = 0.71 # in eV, from Balluffi1978 Table 2
    Ef = 1.28 # in eV, from Balluffi1978 Table 2
    surface_energy = 0.708 # Total guess
  [../]
  [./thcond]
    type = ParsedMaterial
    block = 0
    coupled_variables = 'c'
    expression = 'if(c>0.7,1e-8,4e-8)'
    property_name = thermal_conductivity
    outputs = exodus
  [../]
  [./free_energy]
    type = PolynomialFreeEnergy
    block = 0
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
  scheme = 'bdf2'
  solve_type = 'PJFNK'

  l_max_its = 30
  l_tol = 1.0e-4
  nl_max_its = 25
  nl_rel_tol = 1.0e-9

  num_steps = 60
  dt = 20.0
[]

[Outputs]
   exodus = true
[]
