# Tests physics and energy conservation for HSCoupler2D3D.

R_pipe = 0.005
length_matrix = 0.5
length_extend = 0.6

n_elems_radial = 3
n_elems_axial_matrix = 10
n_elems_axial_extend = 12

[Materials]
  [matrix_mat]
    type = ADGenericConstantMaterial
    block = 'hs3d:0 hs2d:pipe'
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '8000 500 15'
  []
[]

[Functions]
  [initial_T_matrix_fn]
    type = ParsedFunction
    expression = '300 + 100*z - 1000*x'
  []
[]

[Components]
  [hs3d]
    type = HeatStructureFromFile3D
    file = mesh/mesh.e
    position = '0 0 0'
    initial_T = initial_T_matrix_fn
  []

  [hs2d]
    type = HeatStructureCylindrical
    orientation = '0 0 1'
    position = '0 0 0'
    length = '${length_matrix} ${length_extend}'
    n_elems = '${n_elems_axial_matrix} ${n_elems_axial_extend}'
    axial_region_names = 'matrix extend'

    inner_radius = 0
    widths = '${R_pipe}'
    n_part_elems = '${n_elems_radial}'
    names = 'pipe'

    initial_T = 300
  []

  [hs_coupler]
    type = HSCoupler2D3D
    heat_structure_2d = hs2d
    heat_structure_3d = hs3d
    boundary_2d = hs2d:matrix:outer
    boundary_3d = hs3d:rmin

    include_radiation = false
    gap_thickness = 0.00001
    gap_thermal_conductivity = 0.05
  []
[]

[Postprocessors]
  [energy_hs3d]
    type = ADHeatStructureEnergy3D
    block = 'hs3d:0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy_hs2d]
    type = ADHeatStructureEnergyRZ
    block = 'hs2d:pipe'
    axis_dir = '0 0 1'
    axis_point = '0 0 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_energy]
    type = SumPostprocessor
    values = 'energy_hs3d energy_hs2d'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy_change]
    type = ChangeOverTimePostprocessor
    change_with_respect_to_initial = true
    postprocessor = total_energy
    compute_relative_change = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  dt = 0.1
  num_steps = 10

  solve_type = NEWTON
  abort_on_solve_fail = true
  nl_abs_tol = 1e-8
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]
