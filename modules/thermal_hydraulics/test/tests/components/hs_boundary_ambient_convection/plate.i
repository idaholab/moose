T_hs = 300
T_ambient1 = 500
htc1 = 100
T_ambient2 = 400
htc2 = 300
t = 0.001

L = 2
thickness = 0.5
depth = 0.6

# SS 316
density = 8.0272e3
specific_heat_capacity = 502.1
conductivity = 16.26

A = ${fparse L * depth}
heat_flux_avg = ${fparse 0.5 * (htc1 * (T_ambient1 - T_hs) + htc2 * (T_ambient2 - T_hs))}
heat_flux_integral = ${fparse heat_flux_avg * A}
scale = 0.8
E_change = ${fparse scale * heat_flux_integral * t}

[Functions]
  [T_ambient_fn]
    type = PiecewiseConstant
    axis = z
    x = '0 1'
    y = '${T_ambient1} ${T_ambient2}'
  []
  [htc_ambient_fn]
    type = PiecewiseConstant
    axis = z
    x = '0 1'
    y = '${htc1} ${htc2}'
  []
[]

[HeatStructureMaterials]
  [hs_mat]
    type = SolidMaterialProperties
    rho = ${density}
    cp = ${specific_heat_capacity}
    k = ${conductivity}
  []
[]

[Components]
  [hs]
    type = HeatStructurePlate
    orientation = '0 0 1'
    position = '0 0 0'
    length = ${L}
    n_elems = 10

    depth = ${depth}
    widths = '${thickness}'
    n_part_elems = '10'
    materials = 'hs_mat'
    names = 'region'

    initial_T = ${T_hs}
  []

  [ambient_convection]
    type = HSBoundaryAmbientConvection
    boundary = 'hs:outer'
    hs = hs
    T_ambient = T_ambient_fn
    htc_ambient = htc_ambient_fn
    scale = ${scale}
  []
[]

[Postprocessors]
  [E_hs]
    type = ADHeatStructureEnergy
    block = 'hs:region'
    plate_depth = ${depth}
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
