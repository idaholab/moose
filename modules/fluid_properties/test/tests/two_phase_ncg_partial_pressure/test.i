p = 40e3
T = 300

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FluidProperties]
  [fp_air]
    type = IdealGasFluidProperties
  []
  [fp_2phase]
    type = StiffenedGasTwoPhaseFluidProperties
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
