[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[FluidProperties]
  [fp_steam]
    type = StiffenedGasFluidProperties
    gamma = 1.43
    cv = 1040.0
    q = 2.03e6
    p_inf = 0.0
    q_prime = -2.3e4
    k = 0.026
    mu = 134.4e-7
    M = 0.01801488
    rho_c = 322.0
  []
[]

[AuxVariables]
  [rho]
  []
  [p]
  []
  [T]
  []
[]

[ICs]
  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = p
    T = T
    fp = fp_steam
  []
  [p_ic]
    type = ConstantIC
    variable = p
    value = 100e3
  []
  [T_ic]
    type = ConstantIC
    variable = T
    value = 500
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [rho_test]
    type = ElementalVariableValue
    elementid = 0
    variable = rho
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]

[Problem]
  solve = false
[]
