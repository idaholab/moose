starting_point = 2e-1
offset = -0.19

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = long-bottom-block-1elem-blocks.e
[]

[Variables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
  [normal_lm]
    block = 3
    use_dual = true
  []
[]

[ICs]
  [disp_y]
    block = 2
    variable = disp_y
    value = '${fparse starting_point + offset}'
    type = ConstantIC
  []
[]

[Kernels]
  [DynamicTensorMechanics]
    displacements = 'disp_x disp_y'
    generate_output = 'stress_xx stress_yy'
    strain = FINITE
    block = '1 2'
    zeta = 1.0
    alpha = 0.0
  []
  [inertia_x]
    type = InertialForce
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    beta = 0.25
    gamma = 0.5
    alpha = 0
    eta = 0.0
    block = '1 2'
  []
  [inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    beta = 0.25
    gamma = 0.5
    alpha = 0
    eta = 0.0
    block = '1 2'
  []
[]

[Materials]
  [elasticity_2]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [elasticity_1]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e8
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  []
  [strain]
    type = ComputeFiniteStrain
    block = '1 2'
  []
  [density]
    type = GenericConstantMaterial
    block = '1 2'
    prop_names = 'density'
    prop_values = '7750'
  []
[]

[AuxVariables]
  [vel_x]
    block = '1 2'
  []
  [accel_x]
    block = '1 2'
  []
  [vel_y]
    block = '1 2'
  []
  [accel_y]
    block = '1 2'
  []
  [vel_z]
    block = '1 2'
  []
  [accel_z]
    block = '1 2'
  []
  [kinetic_energy]
    order = CONSTANT
    family = MONOMIAL
  []
  [elastic_energy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [accel_x]
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    beta = 0.25
    execute_on = 'timestep_end'
  []
  [vel_x]
    type = NewmarkVelAux
    variable = vel_x
    acceleration = accel_x
    gamma = 0.5
    execute_on = 'timestep_end'
  []
  [accel_y]
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y
    beta = 0.25
    execute_on = 'timestep_end'
  []
  [vel_y]
    type = NewmarkVelAux
    variable = vel_y
    acceleration = accel_y
    gamma = 0.5
    execute_on = 'timestep_end'
  []
  [kinetic_energy]
    type = KineticEnergyAux
    block = '1 2'
    variable = kinetic_energy
    newmark_velocity_x = vel_x
    newmark_velocity_y = vel_y
    newmark_velocity_z = 0.0
    density = density
  []
  [elastic_energy]
    type = ElasticEnergyAux
    variable = elastic_energy
    block = '1 2'
  []
[]


# User object provides the contact force (e.g. LM)
# for the application of the generalized force
[UserObjects]
  [weighted_gap_uo]
    type = LMWeightedGapUserObject
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    lm_variable = normal_lm
    disp_x = disp_x
    disp_y = disp_y
  []
[]


[Constraints]
  # Not using 'dynamic' constraints results in poor enforcement of contact
  # constraints and lack of kinetic and elastic energy conservation.
  [weighted_gap_lm]
    type = ComputeDynamicWeightedGapLMMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = normal_lm
    disp_x = disp_x
    disp_y = disp_y
    newmark_beta = 0.25
    newmark_gamma = 0.5
    use_displaced_mesh = true
    # Capture tolerance is important. If too small, stabilization takes longer
    capture_tolerance = 1.0e-5
    c = 1.0e6
  []

  [normal_x]
    type = NormalMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = normal_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_gap_uo
  []
  [normal_y]
    type = NormalMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = normal_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_gap_uo
  []
[]

[BCs]
  [botx]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    boundary = 40
    value = 0.0
  []
  [topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 30
    function = '${starting_point} * cos(2 * pi / 4 * t) + ${offset}'
  []
  [leftx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 30 # 50
    function = '0' # '1e-2*t'
  []
[]

[Executioner]
  type = Transient
  end_time = 0.275 # 8.0
  dt = 0.025
  dtmin = .025
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       NONZERO               1e-15'
  nl_max_its = 50
  line_search = 'none'
  [TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
  checkpoint = true
  csv = true
[]

[Postprocessors]
  active = 'contact total_kinetic_energy total_elastic_energy'
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
  [contact]
    type = ContactDOFSetSize
    variable = normal_lm
    subdomain = '3'
    execute_on = 'nonlinear timestep_end'
  []
  [total_kinetic_energy]
    type = ElementIntegralVariablePostprocessor
    variable = kinetic_energy
    block = '1 2'
  []
  [total_elastic_energy]
    type = ElementIntegralVariablePostprocessor
    variable = elastic_energy
    block = '1 2'
  []
[]
