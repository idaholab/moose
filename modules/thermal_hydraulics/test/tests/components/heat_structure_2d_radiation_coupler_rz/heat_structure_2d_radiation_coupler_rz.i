emissivity1 = 0.75
emissivity2 = 0.5

[HeatStructureMaterials]
  [hs_mat]
    type = SolidMaterialProperties
    k = 15
    cp = 500
    rho = 8000
  []
[]

[Components]
  [hs1]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 1 0'
    length = 0.5
    n_elems = 25

    inner_radius = 0.1
    names = 'region1'
    widths = '0.1'
    n_part_elems = '5'
    materials = 'hs_mat'

    initial_T = 300
  []

  [hs2]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 1 0'
    length = '0.5 0.5'
    n_elems = '25 25'
    axial_region_names = 'axregion1 axregion2'

    inner_radius = 0.5
    names = 'region1'
    widths = '0.1'
    n_part_elems = '5'
    materials = 'hs_mat'

    initial_T = 1000
  []

  [hs_coupler]
    type = HeatStructure2DRadiationCouplerRZ
    primary_heat_structure = hs1
    secondary_heat_structure = hs2
    primary_boundary = hs1:outer
    secondary_boundary = hs2:axregion1:inner
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
  [E_tot]
    type = ADHeatStructureEnergyRZ
    block = 'hs1:region1 hs2:region1'
    axis_dir = '1 1 0'
    axis_point = '0 0 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_tot_change]
    type = ChangeOverTimePostprocessor
    change_with_respect_to_initial = true
    postprocessor = E_tot
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T1]
    type = SideAverageValue
    variable = T_solid
    boundary = hs1:outer
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T2]
    type = SideAverageValue
    variable = T_solid
    boundary = hs2:axregion1:inner
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
  file_base = 'heat_structure_2d_radiation_coupler_rz'
  [csv]
    type = CSV
    show = 'E_tot_change T1 T2'
  []
[]
