T_hs = 1200
T_ambient = 1500
emissivity = 0.3
view_factor = 0.6
t = 5.0

L = 2
D_i = 0.2
thickness = 0.5

# SS 316
density = 8.0272e3
specific_heat_capacity = 502.1
conductivity = 16.26

stefan_boltzmann = 5.670367e-8
R_i = ${fparse 0.5 * D_i}
D_o = ${fparse D_i + 2 * thickness}
A = ${fparse pi * D_o * L}
heat_flux = ${fparse stefan_boltzmann * emissivity * view_factor * (T_ambient^4 - T_hs^4)}
scale = 0.8
power = ${fparse scale * heat_flux * A}
E_change = ${fparse power * t}

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
    type = HeatStructureCylindrical
    orientation = '0 0 1'
    position = '0 0 0'
    length = ${L}
    n_elems = 10

    inner_radius = ${R_i}
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
    scale = ${scale}
  []
[]

[Postprocessors]
  [E_hs]
    type = ADHeatStructureEnergyRZ
    block = 'hs:region'
    axis_dir = '0 0 1'
    axis_point = '0 0 0'
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

  [heat_rate_pp_relerr]
    type = RelativeDifferencePostprocessor
    value1 = hs_boundary_integral
    value2 = ${power}
    execute_on = 'INITIAL'
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
    show = 'E_change_relerr heat_rate_pp_relerr'
    execute_on = 'FINAL'
  []
[]
