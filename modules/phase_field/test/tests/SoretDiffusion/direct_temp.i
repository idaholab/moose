[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 30
  xmax = 500
  elem_type = EDGE
[]

[GlobalParams]
  polynomial_order = 8
[]

[Variables]
  [./c]
    family = HERMITE
    order = THIRD
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
    coupled_variables = 'c'
    expression = 'if(c>0.7,1e-8,4e-8)'
    property_name = thermal_conductivity
    outputs = exodus
  [../]
  [./free_energy]
    type = PolynomialFreeEnergy
    c = c
    derivative_order = 3
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
  dt = 8.0
[]

[Outputs]
  exodus = true
[]
