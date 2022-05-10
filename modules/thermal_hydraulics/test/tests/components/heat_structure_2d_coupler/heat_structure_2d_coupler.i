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
    position = '-0.5 0 0'
    orientation = '1 0 0'
    length = 0.5
    n_elems = 5

    names = 'region1'
    widths = '0.5'
    n_part_elems = '5'
    materials = 'hs_mat'

    initial_T = 500
  []

  [hs2]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'
    length = '0.5 0.5'
    n_elems = '5 5'
    axial_region_names = 'axregion1 axregion2'

    names = 'region1 region2'
    widths = '0.5 0.2'
    n_part_elems = '5 3'
    materials = 'hs_mat hs_mat'

    initial_T = 300
  []

  [hs3]
    type = HeatStructureCylindrical
    position = '0.5 0 0'
    orientation = '1 0 0'
    length = 0.5
    n_elems = 5

    names = 'region1'
    widths = '0.5'
    n_part_elems = '5'
    materials = 'hs_mat'

    initial_T = 500
  []

  [hs_coupling_1_2]
    type = HeatStructure2DCoupler
    primary_heat_structure = hs2
    secondary_heat_structure = hs1
    primary_boundary = hs2:region1:start
    secondary_boundary = hs1:end
    heat_transfer_coefficient = 1000
  []

  [hs_coupling_2_3]
    type = HeatStructure2DCoupler
    primary_heat_structure = hs2
    secondary_heat_structure = hs3
    primary_boundary = hs2:axregion2:outer
    secondary_boundary = hs3:inner
    heat_transfer_coefficient = 500
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
    block = 'hs1:region1 hs2:region1 hs2:region2 hs3:region1'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 1000
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 30

  l_tol = 1e-4
  l_max_its = 300
[]

[Outputs]
  file_base = 'cylindrical'
  exodus = true
[]
