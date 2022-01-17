T_hs = 300
heat_flux = 1000
t = 0.001

L = 2
thickness = 0.5
depth = 0.6

# SS 316
density = 8.0272e3
specific_heat_capacity = 502.1
conductivity = 16.26

A = ${fparse L * depth}
scale = 0.8
E_change = ${fparse scale * heat_flux * A * t}

[Functions]
  [q_fn]
    type = ConstantFunction
    value = ${heat_flux}
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

  [heat_flux_boundary]
    type = HSBoundaryHeatFlux
    boundary = 'hs:outer'
    hs = hs
    q = q_fn
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
