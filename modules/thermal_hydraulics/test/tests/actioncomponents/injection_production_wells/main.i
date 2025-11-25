
radius = 0.1
A_pipe = ${fparse pi * radius^2}
A_couple = ${A_pipe}
V_junc = ${fparse A_pipe^(3/2)} # cube with A_pipe side area

T_inlet = 300
p_outlet = 1e5
rhou_inlet = 1.0
mdot_inlet = ${fparse A_pipe * rhou_inlet}

inj_surface = '0 0 0'
inj_point1 = '0 0 -2'
inj_point2 = '0 0 -4'

pro_surface = '3 0 0'
pro_point1 = '3 0 -2'
pro_point2 = '3 0 -4'

n_elems = 2

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[ActionComponents]
  [inj]
    type = InjectionWell
    surface_point = ${inj_surface}
    junction_points = '${inj_point1} ${inj_point2}'
    section_n_elems = '${n_elems} ${n_elems}'
    area = ${A_pipe}
    junction_coupling_areas = '${A_couple} ${A_couple}'
    fracture_direction = '1 0 0'
    junction_volume = ${V_junc}
    inlet_mass_flow_rate = ${mdot_inlet}
    inlet_temperature = ${T_inlet}
    initial_pressure = ${p_outlet}
    initial_temperature = ${T_inlet}
    fluid_properties = fp
    multi_app = sub
  []
  [pro]
    type = ProductionWell
    surface_point = ${pro_surface}
    junction_points = '${pro_point1} ${pro_point2}'
    section_n_elems = '${n_elems} ${n_elems}'
    area = ${A_pipe}
    junction_coupling_areas = '${A_couple} ${A_couple}'
    fracture_direction = '-1 0 0'
    junction_volume = ${V_junc}
    outlet_pressure = ${p_outlet}
    initial_pressure = ${p_outlet}
    initial_temperature = ${T_inlet}
    fluid_properties = fp
    multi_app = sub
  []
[]

# This is necessary to trigger some THM actions
[Components]
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = ThermalHydraulicsApp
    input_files = sub.i
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2

  dt = 1
  end_time = 2

  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]
