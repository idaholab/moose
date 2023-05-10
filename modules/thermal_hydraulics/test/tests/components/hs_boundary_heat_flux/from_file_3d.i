T_hs = 300
heat_flux = 1000
t = 0.001

# dimensions of the side 'left'
height = 5
depth = 2

# SS 316
density = 8.0272e3
specific_heat_capacity = 502.1
conductivity = 16.26

A = ${fparse height * depth}
scale = 0.8
E_change = ${fparse scale * heat_flux * A * t}

[Functions]
  [q_fn]
    type = ConstantFunction
    value = ${heat_flux}
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

  [heat_flux_boundary]
    type = HSBoundaryHeatFlux
    boundary = 'hs:left'
    hs = hs
    q = q_fn
    scale = ${scale}
  []
[]

[Postprocessors]
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
