rho_left = 1.162633159
E_left = 2.1502913276e+05
u_left = 100

rho_right = 1.116127833
E_right = 1.7919094397e+05
u_right = 90

[Mesh]
  allow_renumbering = false
  [./cartesian]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 1
    nx = 2
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
  [./rho_u]
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
    function = 'if (x < 0.5, ${rho_left}, ${rho_right})'
  [../]

  [./rho_u_ic]
    type = FunctionIC
    variable = rho_u
    function = 'if (x < 0.5, ${fparse rho_left * u_left}, ${fparse rho_right * u_right})'
  [../]

  [./rho_E_ic]
    type = FunctionIC
    variable = rho_E
    function = 'if (x < 0.5, ${fparse E_left * rho_left}, ${fparse E_right * rho_right})'
  [../]
[]

[Materials]
  [./var_mat]
    type = ConservedVarValuesMaterial
    rho = rho
    rhou = rho_u
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
    side_id = 1
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
