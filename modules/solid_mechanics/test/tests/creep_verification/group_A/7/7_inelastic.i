num_elems = 11
elem_width = ${fparse 100/num_elems}
sample_start = ${fparse 100+elem_width/2}
sample_end = ${fparse 200-elem_width/2}
[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2

  coord_type = RZ

  # x = radial coordinate r
  # y = axial coordinate z
  xmin = 100.0
  xmax = 200.0
  ymin = 0.0
  ymax = 25.0

  nx = ${num_elems}
  ny = 1

  elem_type = QUAD8
[]

[AuxVariables]

  [sigma_rr_recovered]
    order = SECOND
    family = LAGRANGE
  []

  [sigma_zz_recovered]
    order = SECOND
    family = LAGRANGE
  []

  [sigma_hoop_recovered]
    order = SECOND
    family = LAGRANGE
  []

  [creep_rr]
    order = CONSTANT
    family = MONOMIAL
  []
  [creep_zz_axial]
    order = CONSTANT
    family = MONOMIAL
  []
  [creep_tt]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Functions]
  [coefficientFunction]
    type = ParsedFunction
    expression = 'if(t <= 0, 0, 1e-18)'
  []

  # Time unit is hour.
  [pressure_ramp]
    type = ParsedFunction
    expression = '200'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    incremental = true
    add_variables = true
    #use_automatic_differentiation = true
  []
[]

[BCs]
  # Prescribed zero z-displacement on AB: z = 0
  [fix_AB_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []

  # Prescribed zero z-displacement on CD: z = H
  [fix_CD_z]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value = 0.0
  []

  # Internal pressure on the inner radius.
  # Do NOT constrain disp_r on the inner or outer wall.
  [internal_pressure_r]
    type = Pressure
    variable = disp_r
    boundary = left
    function = pressure_ramp
    use_displaced_mesh = false
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.0e5
    poissons_ratio = 0.3
  []

  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'creep'
  []

  [creep]
    type = PowerLawCreepStressUpdate

    # Norton secondary creep:
    #   edot_c = A * sigma_eff^n
    #
    # This coefficient is intentionally accelerated so the stress redistribution
    # reaches steady state in a practical run time. The steady-state stress shape
    # should not depend on A for this secondary-creep benchmark.
    coefficient = coefficientFunction
    n_exponent = 5.0
    m_exponent = 0.0
    activation_energy = 0.0

    use_substepping = INCREMENT_BASED
#    substep_strain_tolerance = 0.05
    max_inelastic_increment = 1.0e-5
    maximum_number_substeps = 50000 #50
    automatic_differentiation_return_mapping = true
  []
[]

[AuxKernels]
  [creep_rr]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_rr
    index_i = 0
    index_j = 0
    execute_on = 'TIMESTEP_END'
  []

  [creep_zz_axial]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_zz_axial
    index_i = 1
    index_j = 1
    execute_on = 'TIMESTEP_END'
  []

  [creep_tt]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_tt
    index_i = 2
    index_j = 2
    execute_on = 'TIMESTEP_END'
  []

  [sigma_rr_recovered]
    type = NodalPatchRecoveryAux
    variable = sigma_rr_recovered
    nodal_patch_recovery_uo = sigma_rr_patch
    execute_on = 'TIMESTEP_END'
  []
  [sigma_hoop_recovered]
    type = NodalPatchRecoveryAux
    variable = sigma_hoop_recovered
    nodal_patch_recovery_uo = sigma_hoop_patch
    execute_on = 'TIMESTEP_END'
  []
  [sigma_zz_recovered]
    type = NodalPatchRecoveryAux
    variable = sigma_zz_recovered
    nodal_patch_recovery_uo = sigma_zz_patch
    execute_on = 'TIMESTEP_END'
  []
[]

[UserObjects]
  [sigma_rr_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = SECOND
    property = 'stress'
    component = '0 0'
    execute_on = 'TIMESTEP_END'
  []
  [sigma_hoop_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = SECOND
    property = 'stress'
    component = '2 2'
    execute_on = 'TIMESTEP_END'
  []
  [sigma_zz_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = SECOND
    property = 'stress'
    component = '1 1'
    execute_on = 'TIMESTEP_END'
  []
[]


[Postprocessors]
  [material_dt]
    type = MaterialTimeStepPostprocessor
  []

  [max_sigma_rr]
    type = ElementExtremeValue
    variable = sigma_rr_recovered
    value_type = max
  []

  [min_sigma_rr]
    type = ElementExtremeValue
    variable = sigma_rr_recovered
    value_type = min
  []

  [max_sigma_hoop]
    type = ElementExtremeValue
    variable = sigma_hoop_recovered
    value_type = max
  []

  [min_sigma_hoop]
    type = ElementExtremeValue
    variable = sigma_hoop_recovered
    value_type = min
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  # Time unit is hour.
  start_time = -1
  end_time = 1e4

  dtmin = 1e-12
  dtmax = 10

  nl_max_its = 50
  l_max_its = 100

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-12

  automatic_scaling = true

  [TimeStepper]
    type = IterationAdaptiveDT

    dt = 1

    optimal_iterations = 8
    iteration_window = 2
    linear_iteration_ratio = 400

    timestep_limiting_postprocessor = material_dt

    time_t = '-1 0'
    time_dt = '1 1e-20'
  []
[]

[VectorPostprocessors]
  # Output final radial profile.
  #
  # sigma_rr   -> radial stress
  # sigma_hoop -> hoop stress
  # sigma_zz   -> axial stress
  [stress_profile]
    type = LineValueSampler
    variable = 'sigma_rr_recovered sigma_hoop_recovered sigma_zz_recovered'
    start_point = '${sample_start} 12.5 0.0'
    end_point = '${sample_end} 12.5 0.0'
    num_points = ${num_elems}
    sort_by = x
    execute_on = final
    outputs = line_csv
  []

  # Same variables at the five comparison radii.
  [stress_points]
    type = LineValueSampler
    variable = 'sigma_rr_recovered sigma_hoop_recovered sigma_zz_recovered'
    start_point = '100.0 12.5 0.0'
    end_point = '200.0 12.5 0.0'
    num_points = 5
    sort_by = x
    execute_on = final
    outputs = point_csv
  []
[]

[Outputs]
  exodus = true
  [console]
    type = Console
    max_rows = 5
  []

  [csv]
    type = CSV
    execute_on = 'initial timestep_end final'
  []

  [line_csv]
    type = CSV
    execute_on = final
    create_final_symlink = true
    file_base = '7_inelastic_profile'
  []

  [point_csv]
    type = CSV
    execute_on = final
    create_final_symlink = true
    file_base = '7_inelastic_points'
  []
[]
