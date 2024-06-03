# This input file is used to test that HSCoupler2D2DRadiation conserves
# energy for a problem where 3 cylindrical heat structures (surfaces 1, 2, and 3)
# are enclosed by an annular heat structure (surface 4). Note that the mesh
# positions used in this input file do not reflect the real positions for this
# configuration, for convenience of viewing solutions.

emissivity1 = 0.8
emissivity2 = 0.5
emissivity3 = 0.2
emissivity4 = 0.9

orientation = '0 0 1'

length = 0.5
n_axial_elems = 10

radius_123 = 0.1
inner_radius_4 = 0.2
outer_radius_4 = 0.25
thickness_4 = ${fparse outer_radius_4 - inner_radius_4}
n_radial_elems_123 = 10
n_radial_elems_4 = 5

initial_T1 = 1200
initial_T2 = 1000
initial_T3 = 800
initial_T4 = 300
T_ref = 300

y_shift = 0.5
position1 = '0 0 0'
position2 = '0 ${y_shift} 0'
position3 = '0 ${fparse 2*y_shift} 0'
position4 = '0 ${fparse 3*y_shift} 0'

view_factor_12 = 0.15 # guessed some number < 1/6
view_factor_14 = ${fparse 1.0 - 2 * view_factor_12}
view_factor_41 = ${fparse radius_123 / inner_radius_4 * view_factor_14}
view_factor_44 = ${fparse 1.0 - 3 * view_factor_41}

[SolidProperties]
  [hs_mat]
    type = ThermalFunctionSolidProperties
    k = 15
    cp = 500
    rho = 8000
  []
[]

[Components]
  [hs1]
    type = HeatStructureCylindrical
    position = ${position1}
    orientation = ${orientation}
    length = ${length}
    n_elems = ${n_axial_elems}

    names = 'body'
    widths = '${radius_123}'
    n_part_elems = '${n_radial_elems_123}'
    solid_properties = 'hs_mat'
    solid_properties_T_ref = '${T_ref}'

    initial_T = ${initial_T1}
  []
  [hs2]
    type = HeatStructureCylindrical
    position = ${position2}
    orientation = ${orientation}
    length = ${length}
    n_elems = ${n_axial_elems}

    names = 'body'
    widths = '${radius_123}'
    n_part_elems = '${n_radial_elems_123}'
    solid_properties = 'hs_mat'
    solid_properties_T_ref = '${T_ref}'

    initial_T = ${initial_T2}
  []
  [hs3]
    type = HeatStructureCylindrical
    position = ${position3}
    orientation = ${orientation}
    length = ${length}
    n_elems = ${n_axial_elems}

    names = 'body'
    widths = '${radius_123}'
    n_part_elems = '${n_radial_elems_123}'
    solid_properties = 'hs_mat'
    solid_properties_T_ref = '${T_ref}'

    initial_T = ${initial_T3}
  []
  [hs4]
    type = HeatStructureCylindrical
    position = ${position4}
    orientation = ${orientation}
    length = ${length}
    n_elems = ${n_axial_elems}

    inner_radius = ${inner_radius_4}
    names = 'body'
    widths = '${thickness_4}'
    n_part_elems = '${n_radial_elems_4}'
    solid_properties = 'hs_mat'
    solid_properties_T_ref = '${T_ref}'

    initial_T = ${initial_T4}
  []
  [hs_coupler]
    type = HSCoupler2D2DRadiation
    heat_structures = 'hs1 hs2 hs3 hs4'
    boundaries = 'hs1:outer hs2:outer hs3:outer hs4:inner'
    emissivities = '${emissivity1} ${emissivity2} ${emissivity3} ${emissivity4}'
    include_environment = false
    view_factors = '
      0 ${view_factor_12} ${view_factor_12} ${view_factor_14};
      ${view_factor_12} 0 ${view_factor_12} ${view_factor_14};
      ${view_factor_12} ${view_factor_12} 0 ${view_factor_14};
      ${view_factor_41} ${view_factor_41} ${view_factor_41} ${view_factor_44}'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [E_hs1]
    type = ADHeatStructureEnergyRZ
    block = 'hs1:body'
    axis_dir = ${orientation}
    axis_point = ${position1}
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_hs2]
    type = ADHeatStructureEnergyRZ
    block = 'hs2:body'
    axis_dir = ${orientation}
    axis_point = ${position2}
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_hs3]
    type = ADHeatStructureEnergyRZ
    block = 'hs3:body'
    axis_dir = ${orientation}
    axis_point = ${position3}
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_hs4]
    type = ADHeatStructureEnergyRZ
    block = 'hs4:body'
    axis_dir = ${orientation}
    axis_point = ${position4}
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_tot]
    type = ParsedPostprocessor
    expression = 'E_hs1 + E_hs2 + E_hs3 + E_hs4'
    pp_names = 'E_hs1 E_hs2 E_hs3 E_hs4'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_tot_err]
    type = ChangeOverTimePostprocessor
    postprocessor = E_tot
    take_absolute_value = true
    change_with_respect_to_initial = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 10
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  nl_max_its = 10

  l_tol = 1e-4
  l_max_its = 10
[]

[Outputs]
  file_base = 'energy_conservation'
  [csv]
    type = CSV
    show = 'E_tot_err'
    execute_on = 'FINAL'
  []
[]
