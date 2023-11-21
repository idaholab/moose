[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
  elem_type = HEX8
[]

[AuxVariables]
  [slip_resistance_0]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_resistance_3]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_resistance_6]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_resistance_9]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  incremental = true
  add_variables = true
  generate_output = stress_zz
[]

[AuxKernels]
  [slip_resistance_0]
    type = MaterialStdVectorAux
    variable = slip_resistance_0
    property = slip_resistance
    index = 0
    execute_on = timestep_end
  []
  [slip_resistance_3]
    type = MaterialStdVectorAux
    variable = slip_resistance_3
    property = slip_resistance
    index = 3
    execute_on = timestep_end
  []
  [slip_resistance_6]
    type = MaterialStdVectorAux
    variable = slip_resistance_6
    property = slip_resistance
    index = 6
    execute_on = timestep_end
  []
  [slip_resistance_9]
    type = MaterialStdVectorAux
    variable = slip_resistance_9
    property = slip_resistance
    index = 9
    execute_on = timestep_end
  []
[]

[BCs]
  [symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [constant_displacement]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = '1.0e-3*t'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.98e5 1.25e5 1.25e5 1.98e5 1.25e5 1.98e5 1.22e5 1.22e5 1.22e5'
    fill_method = symmetric9
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'trial_xtalpl'
    tan_mod_type = exact
    line_search_method = CUT_HALF
    use_line_search = true
    maximum_substep_iteration = 5
  []
  [trial_xtalpl]
    type = CrystalPlasticityFCCDislocationLinkHuCocksUpdate
    number_slip_systems = 12
    slip_sys_file_name = input_slip_sys.txt
    number_coplanar_groups = 4
    shear_modulus = 1.22e5 #in MPa, from Hu et al 2016 IJP
    burgers_vector = 2.5e-7 #in mm, from Hu et al 2016 IJP
    initial_pinning_point_density = 3.16e7 ## prescribed for unit test
    coefficient_self_plane_evolution = 5.0e8 ## in 1/mm^2, from Hu et al 2016 IJP
    coefficient_latent_plane_evolution = 2.5e9 ## in 1/mm^2, from Hu et al 2016 IJP
    forest_dislocation_hardening_coefficient = 0.0 #unitless, placeholder
    solute_hardening_coefficient = 0.00457 #unitless, Hu et al 2016 IJP
    # solute_concentration = solute_conc
    # precipitate_number_density = precipitate_conc
    # mean_precipitate_radius = precipitate_radius
    precipitate_hardening_coefficient = 0.84 #unitless, Foreman and Makin 1966 via Hu et al 2016 IJP
    stol = 5.0e-3
    resistance_tol = 5.0e-3
    zero_tol = 1e-16
  []
  [concentrations]
    type = GenericConstantMaterial
    prop_names = 'solute_conc  precipitate_conc  precipitate_radius'
    prop_values = '1.287e17       1.0e10            7.62e-6'
  []
[]

[Postprocessors]
  [slip_resistance_0]
    type = ElementAverageValue
    variable = slip_resistance_0
  []
  [slip_resistance_3]
    type = ElementAverageValue
    variable = slip_resistance_3
  []
  [slip_resistance_6]
    type = ElementAverageValue
    variable = slip_resistance_6
  []
  [slip_resistance_9]
    type = ElementAverageValue
    variable = slip_resistance_9
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'
  nl_abs_tol = 1e-14
  nl_rel_tol = 1e-10
  nl_abs_step_tol = 1e-10
  nl_max_its = 15

  dt = 0.1
  dtmin = 1e-4
  num_steps = 1
[]

[Outputs]
  csv = true
  console = true
  perf_graph = true
[]
