T_hs = 1200
T_ambient = 1500
emissivity = 0.3
view_factor = 0.6
t = 5.0

# dimensions of the side 'left'
height = 5
depth = 2

# SS 316
density = 8.0272e3
specific_heat_capacity = 502.1
conductivity = 16.26

stefan_boltzmann = 5.670367e-8
A = ${fparse height * depth}
heat_flux = ${fparse stefan_boltzmann * emissivity * view_factor * (T_ambient^4 - T_hs^4)}
scale = 0.8
E_change = ${fparse scale * heat_flux * A * t}

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

  [hs_boundary]
    type = HSBoundaryRadiation
    boundary = 'hs:left'
    hs = hs
    T_ambient = ${T_ambient}
    emissivity = ${emissivity}
    view_factor = ${view_factor}
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
