T_hs = 300
T_ambient1 = 500
htc1 = 100
T_ambient2 = 400
htc2 = 300
t = 0.001

# dimensions of the side 'left'
height = 5
depth = 2

# SS 316
density = 8.0272e3
specific_heat_capacity = 502.1
conductivity = 16.26

A = ${fparse height * depth}
heat_flux_avg = ${fparse 0.5 * (htc1 * (T_ambient1 - T_hs) + htc2 * (T_ambient2 - T_hs))}
heat_flux_integral = ${fparse heat_flux_avg * A}
scale = 0.8
E_change = ${fparse scale * heat_flux_integral * t}

[Functions]
  [T_ambient_fn]
    type = PiecewiseConstant
    axis = z
    x = '-2.5 0'
    y = '${T_ambient1} ${T_ambient2}'
  []
  [htc_ambient_fn]
    type = PiecewiseConstant
    axis = z
    x = '-2.5 0'
    y = '${htc1} ${htc2}'
  []
[]

[Materials]
  [mat]
    type = ADGenericConstantMaterial
    block = 'hs:brick'
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '${density} ${specific_heat_capacity} ${conductivity}'
  []
[]

[Components]
  [hs]
    type = HeatStructureFromFile3D
    file = box.e
    position = '0 0 0'
    initial_T = ${T_hs}
  []

  [ambient_convection]
    type = HSBoundaryAmbientConvection
    boundary = 'hs:left'
    hs = hs
    T_ambient = T_ambient_fn
    htc_ambient = htc_ambient_fn
    scale_pp = bc_scale_pp
  []
[]

[Postprocessors]
  [bc_scale_pp]
    type = FunctionValuePostprocessor
    function = ${scale}
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_hs]
    type = ADHeatStructureEnergy3D
    block = 'hs:brick'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_hs_change]
    type = ChangeOverTimePostprocessor
    postprocessor = E_hs
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_change_relerr]
    type = RelativeDifferencePostprocessor
    value1 = E_hs_change
    value2 = ${E_change}
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient

  [TimeIntegrator]
    type = ActuallyExplicitEuler
    solve_type = lumped
  []
  dt = ${t}
  num_steps = 1
  abort_on_solve_fail = true
[]

[Outputs]
  [out]
    type = CSV
    show = 'E_change_relerr'
    execute_on = 'FINAL'
  []
[]
