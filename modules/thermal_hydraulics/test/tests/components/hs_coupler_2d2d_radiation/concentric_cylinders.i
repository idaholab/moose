# This input file is used to test that HSCoupler2D2DRadiation produces
# the exact same heat fluxes as HeatStructure2DRadiationCouplerRZ for the case
# of two concentric cylindrical heat structures forming an enclosure.
#
# We solve two independent problems, one using HSCoupler2D2DRadiation, and
# the other using HeatStructure2DRadiationCouplerRZ.

emissivity1 = 0.75
emissivity2 = 0.5

orientation = '0 0 1'

length = 0.5
n_axial_elems = 10

outer_radius1 = 0.1
inner_radius2 = 0.15
outer_radius2 = 0.2
thickness2 = ${fparse outer_radius2 - inner_radius2}
n_radial_elems1 = 10
n_radial_elems2 = 5

initial_T1 = 300
initial_T2 = 1000
T_ref = 300

y_shiftB = 0.5

view_factor21 = ${fparse outer_radius1 / inner_radius2}
view_factor22 = ${fparse 1.0 - view_factor21}

[SolidProperties]
  [hs_mat]
    type = ThermalFunctionSolidProperties
    k = 15
    cp = 500
    rho = 8000
  []
[]

[Components]
  # Setup with HSCoupler2D2DRadiation

  [hs1A]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = ${orientation}
    length = ${length}
    n_elems = ${n_axial_elems}

    names = 'region1'
    widths = '${outer_radius1}'
    n_part_elems = '${n_radial_elems1}'
    solid_properties = 'hs_mat'
    solid_properties_T_ref = '${T_ref}'

    initial_T = ${initial_T1}
  []
  [hs2A]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = ${orientation}
    length = ${length}
    n_elems = ${n_axial_elems}

    inner_radius = ${inner_radius2}
    names = 'region1'
    widths = '${thickness2}'
    n_part_elems = '${n_radial_elems2}'
    solid_properties = 'hs_mat'
    solid_properties_T_ref = '${T_ref}'

    initial_T = ${initial_T2}
  []
  [hs_couplerA]
    type = HSCoupler2D2DRadiation
    heat_structures = 'hs1A hs2A'
    boundaries = 'hs1A:outer hs2A:inner'
    emissivities = '${emissivity1} ${emissivity2}'
    include_environment = false
    view_factors = '0.0 1.0; ${view_factor21} ${view_factor22}'
  []

  # Setup with HeatStructure2DRadiationCouplerRZ

  [hs1B]
    type = HeatStructureCylindrical
    position = '0 ${y_shiftB} 0'
    orientation = ${orientation}
    length = ${length}
    n_elems = ${n_axial_elems}

    names = 'region1'
    widths = '${outer_radius1}'
    n_part_elems = '${n_radial_elems1}'
    solid_properties = 'hs_mat'
    solid_properties_T_ref = '${T_ref}'

    initial_T = ${initial_T1}
  []
  [hs2B]
    type = HeatStructureCylindrical
    position = '0 ${y_shiftB} 0'
    orientation = ${orientation}
    length = ${length}
    n_elems = ${n_axial_elems}

    inner_radius = ${inner_radius2}
    names = 'region1'
    widths = '${thickness2}'
    n_part_elems = '${n_radial_elems2}'
    solid_properties = 'hs_mat'
    solid_properties_T_ref = '${T_ref}'

    initial_T = ${initial_T2}
  []
  [hs_couplerB]
    type = HeatStructure2DRadiationCouplerRZ
    primary_heat_structure = hs1B
    secondary_heat_structure = hs2B
    primary_boundary = hs1B:outer
    secondary_boundary = hs2B:inner
    primary_emissivity = ${emissivity1}
    secondary_emissivity = ${emissivity2}
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [T1A]
    type = SideAverageValue
    variable = T_solid
    boundary = hs1A:outer
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T2A]
    type = SideAverageValue
    variable = T_solid
    boundary = hs2A:inner
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [T1B]
    type = SideAverageValue
    variable = T_solid
    boundary = hs1B:outer
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T2B]
    type = SideAverageValue
    variable = T_solid
    boundary = hs2B:inner
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [T1_relerr]
    type = RelativeDifferencePostprocessor
    value1 = T1A
    value2 = T1B
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T2_relerr]
    type = RelativeDifferencePostprocessor
    value1 = T2A
    value2 = T2B
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
  file_base = 'concentric_cylinders'
  [csv]
    type = CSV
    show = 'T1_relerr T2_relerr'
    execute_on = 'FINAL'
  []
[]
