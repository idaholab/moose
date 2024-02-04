
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    elem_type = HEX27
  []
[]

[AuxVariables]
  [temperature]
    order = FIRST
    family = LAGRANGE
  []
  [linear_void_strain]
    order = CONSTANT
    family = MONOMIAL
  []
  [e_void_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [e_void_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [e_void_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [f_void_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [pk2_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [fp_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [tau_0]
    order = FIRST
    family = MONOMIAL
  []
  [tau_10]
    order = FIRST
    family = MONOMIAL
  []
[]

[Physics/SolidMechanics/QuasiStatic/all]
  strain = FINITE
  incremental = true
  add_variables = true
[]

[Functions]
  [temperature_ramp]
    type = ParsedFunction
    expression = '600.0 + t'
  []
[]

[AuxKernels]
  [temperature]
    type = FunctionAux
    variable = temperature
    function = 'temperature_ramp'
    execute_on = timestep_begin
  []
  [linear_void_strain]
    type = MaterialRealAux
    variable = linear_void_strain
    property = equivalent_linear_change
    execute_on = timestep_end
  []
  [e_void_xx]
    type = RankTwoAux
    variable = e_void_xx
    rank_two_tensor = void_eigenstrain
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  []
  [e_void_yy]
    type = RankTwoAux
    variable = e_void_yy
    rank_two_tensor = void_eigenstrain
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  []
  [e_void_zz]
    type = RankTwoAux
    variable = e_void_zz
    rank_two_tensor = void_eigenstrain
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [f_void_zz]
    type = RankTwoAux
    variable = f_void_zz
    rank_two_tensor = volumetric_deformation_gradient
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
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
  [tau_0]
    type = MaterialStdVectorAux
    variable = tau_0
    property = applied_shear_stress
    index = 0
    execute_on = timestep_end
  []
  [tau_10]
    type = MaterialStdVectorAux
    variable = tau_10
    property = applied_shear_stress
    index = 10
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
  [hold_front]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 0
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
    eigenstrain_names = void_eigenstrain
    tan_mod_type = exact
    line_search_method = CUT_HALF
    use_line_search = true
    maximum_substep_iteration = 5
  []

  [trial_xtalpl]
    type = CrystalPlasticityKalidindiUpdate
    number_slip_systems = 12
    slip_sys_file_name = input_slip_sys.txt
  []
  [void_eigenstrain]
    type = ComputeCrystalPlasticityVolumetricEigenstrain
    eigenstrain_name = void_eigenstrain
    deformation_gradient_name = volumetric_deformation_gradient
    mean_spherical_void_radius = void_radius
    spherical_void_number_density = void_density
  []
  [void_density]
    type = ParsedMaterial
    property_name = void_density
    coupled_variables = temperature
    expression = '1.0e8 * (temperature - 600.0)'
  []
  [void_radius]
    type = GenericConstantMaterial
    prop_names = void_radius
    prop_values = '1.0e-6'  ##1 nm avg particle radius
  []
[]

[Postprocessors]
  [linear_void_strain]
    type = ElementAverageValue
    variable = linear_void_strain
  []
  [e_void_xx]
    type = ElementAverageValue
    variable = e_void_xx
  []
  [e_void_yy]
    type = ElementAverageValue
    variable = e_void_yy
  []
  [e_void_zz]
    type = ElementAverageValue
    variable = e_void_zz
  []
  [f_void_zz]
    type = ElementAverageValue
    variable = f_void_zz
  []
  [density]
    type = ElementAverageMaterialProperty
    mat_prop = void_density
    execute_on = TIMESTEP_END
  []
  [radius]
    type = ElementAverageMaterialProperty
    mat_prop = void_radius
    execute_on = TIMESTEP_END
  []
  [pk2_zz]
   type = ElementAverageValue
   variable = pk2_zz
  []
  [fp_zz]
    type = ElementAverageValue
    variable = fp_zz
  []
  [tau_0]
    type = ElementAverageValue
    variable = tau_0
  []
  [tau_10]
    type = ElementAverageValue
    variable = tau_10
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

  petsc_options = '-snes_converged_reason'
  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'

  line_search = 'none'
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-8
  nl_forced_its = 1

  dt = 1.0
  dtmin = 0.1
  end_time  = 5.0
[]

[Outputs]
  csv = true
  perf_graph = true
[]
