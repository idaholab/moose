rho_left = 1.162633159
E_left = 2.1502913276e+05
v_left = 40

rho_right = 1.116127833
E_right = 1.7919094397e+05
v_right = 50

[Mesh]
  [./cartesian]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 2
    nx = 1
    ny = 1
    elem_type = 'TRI3'
  [../]
[]

[FluidProperties]
  [./fp]
    type = IdealGasFluidProperties
    allow_imperfect_jacobians = true
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./rho]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./rho_v]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./rho_E]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[ICs]
  [./rho_ic]
    type = FunctionIC
    variable = rho
    function = 'if (y / (2 * x) < 0.5, ${rho_left}, ${rho_right})'
  [../]

  [./rho_v_ic]
    type = FunctionIC
    variable = rho_v
    function = 'if (y / (2 * x) < 0.5, ${fparse rho_left * v_left}, ${fparse rho_right * v_right})'
  [../]

  [./rho_E_ic]
    type = FunctionIC
    variable = rho_E
    function = 'if (y / (2 * x) < 0.5, ${fparse E_left * rho_left}, ${fparse E_right * rho_right})'
  [../]
[]

[Materials]
  [./var_mat]
    type = ConservedVarValuesMaterial
    rho = rho
    rhou = 0
    rhov = rho_v
    rho_et = rho_E
    fp = fp
  [../]
[]

[UserObjects]
  [./hllc]
    type = HLLCUserObject
    fp = fp
  [../]
[]

[VectorPostprocessors]
  [./wave_speeds]
    type = WaveSpeedVPP
    hllc_uo = hllc
    elem_id = 0
    side_id = 2
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
