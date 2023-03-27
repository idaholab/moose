# Tests HeatStructure2DCoupler when the heat structures are separated by some
# distance. The first heat structure has a larger coupling surface than the
# second heat structure. The component will be used to model a given energy
# transfer rate per unit temperature difference [W/K]. This test checks that:
#   a) heat transfer occurs in the correct direction
#   b) energy is conserved
#
# With a goal of transferring 5 W/K and a temperature difference of 200 K, and
# a transient time of 10 seconds, ~10 kJ should be transferred. Note that this
# estimate will not be exact since the temperature difference changes slightly
# over the transient.

initial_T1 = 500
initial_T2 = 300

R1 = 0.1
R2 = 0.05

P2 = ${fparse 2 * pi * R2}

power_per_K = 5.0
L_hs = 0.5
htc = ${fparse power_per_K / (L_hs * P2)}

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
    orientation = '1 0 0'
    length = '${L_hs}'
    n_elems = '5'

    names = 'region1'
    widths = '${R1}'
    n_part_elems = '5'
    materials = 'hs_mat'

    initial_T = ${initial_T1}
  []

  [hs2]
    type = HeatStructureCylindrical
    position = '0 0.3 0'
    orientation = '1 0 0'
    length = '${L_hs}'
    n_elems = '5'

    names = 'region1'
    widths = '${R2}'
    n_part_elems = '5'
    materials = 'hs_mat'

    initial_T = ${initial_T2}
  []

  [hs_coupling]
    type = HeatStructure2DCoupler
    primary_heat_structure = hs1
    secondary_heat_structure = hs2
    primary_boundary = hs1:outer
    secondary_boundary = hs2:outer
    heat_transfer_coefficient = ${htc}
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
    block = 'hs1:region1'
    axis_dir = '1 0 0'
    axis_point = '0 0 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_hs1_change]
    type = ChangeOverTimePostprocessor
    postprocessor = E_hs1
    change_with_respect_to_initial = true
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [E_hs2]
    type = ADHeatStructureEnergyRZ
    block = 'hs2:region1'
    axis_dir = '1 0 0'
    axis_point = '0 0.3 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_hs2_change]
    type = ChangeOverTimePostprocessor
    postprocessor = E_hs2
    change_with_respect_to_initial = true
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [E_tot]
    type = SumPostprocessor
    values = 'E_hs1 E_hs2'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_tot_change]
    type = ChangeOverTimePostprocessor
    postprocessor = E_tot
    change_with_respect_to_initial = true
    compute_relative_change = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 1.0
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 30

  l_tol = 1e-4
  l_max_its = 300
[]

[Outputs]
  csv = true
  show = 'E_hs1_change E_hs2_change E_tot_change'
[]
