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
  [pk2_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [fp_zz]
    order = CONSTANT
    family = MONOMIAL
  []
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
  [pk2_zz]
    type = RankTwoAux
    variable = pk2_zz
    rank_two_tensor = second_piola_kirchhoff_stress
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [fp_zz]
    type = RankTwoAux
    variable = fp_zz
    rank_two_tensor = plastic_deformation_gradient
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
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

[Functions]
  [precipitate_radius_function]
    type = ParsedFunction
    expression = '1.0e-3 + 1.0e-4*t'
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
    maximum_substep_iteration = 1
  []
  [trial_xtalpl]
    type = CrystalPlasticityFCCDislocationLinkHuCocksUpdate
    number_slip_systems = 12
    slip_sys_file_name = input_slip_sys.txt
    number_coplanar_groups = 4
    shear_modulus = 1.22e5 #in MPa, from Hu et al 2016 IJP
    burgers_vector = 2.5e-7 #in mm, from Hu et al 2016 IJP
    initial_pinning_point_density = 2.4e7 ## in 1/mm^2, from Hu et al 2016 IJP
    coefficient_self_plane_evolution = 5.0e8 ## in 1/mm^2, from Hu et al 2016 IJP
    coefficient_latent_plane_evolution = 2.5e9 ## in 1/mm^2, from Hu et al 2016 IJP
    forest_dislocation_hardening_coefficient = 0.35 #unitless, Madec et al 2002 via Hu et al 2016 IJP
    solute_hardening_coefficient = 0.00457 #unitless, Hu et al 2016 IJP
    solute_concentration = solute_conc
    precipitate_number_density = precipitate_conc
    mean_precipitate_radius = precipitate_radius
    precipitate_hardening_coefficient = 0.84 #unitless, Foreman and Makin 1966 via Hu et al 2016 IJP
    stol = 5.0e-3
    resistance_tol = 5.0e-3
    zero_tol = 1e-16
  []
  [concentrations]
    type = GenericConstantMaterial
    prop_names = 'solute_conc             precipitate_conc'
    prop_values = '2.818586455498711e+17  597211024.5155126' # in 1/mm^3, backed out from given stress
  []
  [varying_solute_concentration]
    type = GenericFunctionMaterial
    prop_names = 'precipitate_radius'
    prop_values = precipitate_radius_function
  []
[]

[Postprocessors]
  [pk2_zz]
    type = ElementAverageValue
    variable = pk2_zz
  []
  [fp_zz]
    type = ElementAverageValue
    variable = fp_zz
  []
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
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-8
  nl_abs_step_tol = 1e-10
  nl_max_its = 15

  dt = 0.1
  dtmin = 1e-4
  num_steps = 5
[]

[Outputs]
  csv = true
  console = true
  perf_graph = true
[]
