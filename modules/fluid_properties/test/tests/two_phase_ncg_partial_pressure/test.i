p = 40e3
T = 300

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FluidProperties]
  [fp_water]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
    k = 0.5
    mu = 281.8e-6
    M = 0.01801488
  []
  [fp_steam]
    type = IdealGasFluidProperties
    gamma = 1.43
    molar_mass = 0.01801488
  []
  [fp_air]
    type = IdealGasFluidProperties
  []
  [fp_2phase]
    type = TestTwoPhaseFluidProperties
    fp_liquid = fp_water
    fp_vapor = fp_steam
  []
  [fp_2phase_ncg]
    type = TwoPhaseNCGPartialPressureFluidProperties
    fp_2phase = fp_2phase
    fp_ncg = fp_air
  []
[]

[Functions]
  [p_sat_fn]
    type = TwoPhaseNCGPartialPressureFunction
    fluid_properties = fp_2phase_ncg
    property_call = p_sat
    arg1 = ${T}
  []
  [x_sat_ncg_fn]
    type = TwoPhaseNCGPartialPressureFunction
    fluid_properties = fp_2phase_ncg
    property_call = x_sat_ncg_from_p_T
    arg1 = ${p}
    arg2 = ${T}
  []
[]

[Postprocessors]
  [p_sat]
    type = FunctionValuePostprocessor
    function = p_sat_fn
    execute_on = 'INITIAL'
  []
  [x_sat_ncg]
    type = FunctionValuePostprocessor
    function = x_sat_ncg_fn
    execute_on = 'INITIAL'
  []
[]

[Problem]
  solve = false
[]


[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
