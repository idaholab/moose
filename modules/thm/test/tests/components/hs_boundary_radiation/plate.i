T_hs = 1200
T_ambient = 1500
emissivity = 0.3
view_factor = 0.6
t = 5.0

L = 2
thickness = 0.5
depth = 0.6

# SS 316
density = 8.0272e3
specific_heat_capacity = 502.1
conductivity = 16.26

stefan_boltzmann = 5.670367e-8
A = ${fparse L * depth}
heat_flux = ${fparse stefan_boltzmann * emissivity * view_factor * (T_ambient^4 - T_hs^4)}
scale = 0.8
E_change = ${fparse scale * heat_flux * A * t}

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

  [hs_boundary]
    type = HSBoundaryRadiation
    boundary = 'hs:outer'
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
