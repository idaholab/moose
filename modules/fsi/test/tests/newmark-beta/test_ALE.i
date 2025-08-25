beta = 0.25
gamma = 0.5
eta = 19.63
zeta = 0.000025
youngs_modulus = 1e8

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = tmesh_HR.msh
  []
  [convert]
    type = ElementOrderConversionGenerator
    input = file
    conversion_type = FIRST_ORDER
  []
  [matrix_side_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = convert
    new_boundary = interface_matrix_side
    paired_block = 'inclusion'
    primary_block = 'matrix'
  []
[]

[Variables]
  [disp_x]
    scaling = '${fparse 1/youngs_modulus}'
  []
  [disp_y]
    scaling = '${fparse 1/youngs_modulus}'
  []
  [vel]
    family = LAGRANGE_VEC
    block = 'matrix'
  []
  [p]
    block = 'matrix'
  []
  [lambda]
    family = SCALAR
    block = 'matrix'
  []
[]

[AuxVariables]
  [accel_x]
    block = 'inclusion'
  []
  [accel_y]
    block = 'inclusion'
  []
  [vel_x_solid]
    block = 'inclusion'
  []
  [vel_y_solid]
    block = 'inclusion'
  []
  [vel_x_fluid]
    block = 'matrix'
  []
  [vel_y_fluid]
    block = 'matrix'
  []
[]

[AuxKernels]
  [accel_x] # Calculates and stores acceleration at the end of time step
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x_solid
    beta = ${beta}
    execute_on = timestep_end
    block = 'inclusion'
  []
  [accel_y] # Calculates and stores acceleration at the end of time step
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y_solid
    beta = ${beta}
    execute_on = timestep_end
    block = 'inclusion'
  []
  [vel_x_solid]
    type = NewmarkVelAux
    variable = vel_x_solid
    acceleration = accel_x
    gamma = ${gamma}
    execute_on = timestep_end
    block = 'inclusion'
  []
  [vel_y_solid]
    type = NewmarkVelAux
    variable = vel_y_solid
    acceleration = accel_y
    gamma = ${gamma}
    execute_on = timestep_end
    block = 'inclusion'
  []
  [vel_x_fluid]
    type = VectorVariableComponentAux
    variable = vel_x_fluid
    vector_variable = vel
    execute_on = timestep_end
    component = 'x'
  []
  [vel_y_fluid]
    type = VectorVariableComponentAux
    variable = vel_y_fluid
    vector_variable = vel
    execute_on = timestep_end
    component = 'y'
  []
[]

[ScalarKernels]
  [mean_zero_pressure_lm]
    type = AverageValueConstraint
    variable = lambda
    pp_name = pressure_integral
    value = 0
  []
[]

[Kernels]
  [mat_disp_x]
    type = MatDiffusion
    variable = disp_x
    block = 'matrix'
    use_displaced_mesh = false
    diffusivity = ${youngs_modulus}
  []
  [mat_disp_y]
    type = MatDiffusion
    variable = disp_y
    block = 'matrix'
    use_displaced_mesh = false
    diffusivity = ${youngs_modulus}
  []

  [mass]
    type = INSADMass
    variable = p
    use_displaced_mesh = true
    block = 'matrix'
  []
  [mass_pspg]
    type = INSADMassPSPG
    variable = p
    use_displaced_mesh = true
    block = 'matrix'
  []

  [momentum_time]
    type = INSADMomentumTimeDerivative
    variable = vel
    block = 'matrix'
  []

  [momentum_convection]
    type = INSADMomentumAdvection
    variable = vel
    block = 'matrix'
  []

  [momentum_viscous]
    type = INSADMomentumViscous
    variable = vel
    use_displaced_mesh = true
    block = 'matrix'
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = vel
    pressure = p
    integrate_p_by_parts = true
    use_displaced_mesh = true
    block = 'matrix'
  []

  [momentum_supg]
    type = INSADMomentumSUPG
    variable = vel
    material_velocity = relative_velocity
    block = 'matrix'
    use_displaced_mesh = true
  []

  [momentum_mesh_advection]
    type = INSADMomentumMeshAdvection
    variable = vel
    disp_x = 'disp_x'
    disp_y = 'disp_y'
    use_displaced_mesh = true
    block = 'matrix'
  []

  [mean_zero_pressure]
    type = ScalarLagrangeMultiplier
    variable = p
    lambda = lambda
    block = 'matrix'
  []

  ## Solid Mech Dynamics legacy action (doesn't add InertialForce)
  [DynamicSolidMechanics] # zeta*K*vel + K * disp
    displacements = 'disp_x disp_y'
    stiffness_damping_coefficient = ${zeta}
    block = 'inclusion'
  []

  [inertia_x] # M*accel + eta*M*vel
    type = InertialForce
    variable = disp_x
    velocity = vel_x_solid
    acceleration = accel_x
    beta = ${beta} # Newmark time integration
    gamma = ${gamma} # Newmark time integration
    eta = ${eta}
    block = 'inclusion'
  []

  [inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_x_solid
    acceleration = accel_y
    beta = ${beta}
    gamma = ${gamma}
    eta = ${eta}
    block = 'inclusion'
  []
[]

[InterfaceKernels]
  [penalty]
    type = ADPenaltyVelocityContinuityNewmarkBeta
    variable = vel
    fluid_velocity = vel
    displacements = 'disp_x disp_y'
    solid_velocities = 'vel_x_solid vel_y_solid'
    solid_accelerations = 'accel_x accel_y'
    boundary = 'interface_matrix_side'
    penalty = ${youngs_modulus}
    beta = ${beta}
    gamma = ${gamma}
  []
[]

[Materials]
  [viscous_mat]
    type = ADGenericConstantMaterial
    block = 'matrix'
    prop_names = 'rho mu'
    prop_values = '1  1'
  []

  [ins_mat]
    type = INSADTauMaterial
    velocity = vel
    pressure = p
    block = 'matrix'
  []

  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = ${youngs_modulus}
    poissons_ratio = 0.3
    block = 'inclusion'
  []

  [strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y'
    block = 'inclusion'
  []

  [small_stress]
    type = ComputeFiniteStrainElasticStress
    block = 'inclusion'
  []

  [density]
    type = GenericConstantMaterial
    block = 'inclusion'
    prop_names = density
    prop_values = 3 # kg/m3
  []
[]

[BCs] # mesh boundaries remain still so I dont think we need to use deformed mesh for vel
  [no_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'bottom top left right'
    value = 0
  []
  [no_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom top left right'
    value = 0
  []

  [shear_top_x]
    type = ADVectorFunctionDirichletBC
    boundary = top
    variable = vel
    function_x = '-0.001'
  []

  [shear_bottom_x]
    type = ADVectorFunctionDirichletBC
    boundary = 'bottom'
    variable = vel
    function_x = '0.001'
  []

  [Periodic]
    [vel]
      variable = vel
      primary = 'left'
      secondary = 'right'
      translation = '1 0 0'
    []

    [x_p]
      variable = p
      primary = 'left'
      secondary = 'right'
      translation = '1 0 0'
    []
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
    petsc_options_value = 'lu       NONZERO               strumpack'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  end_time = 100.0
  nl_abs_tol = 1e-12
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 5
    dt = 0.005
    growth_factor = 1.5
    cutback_factor = 0.9
  []
[]

[Postprocessors]
  [pressure_integral]
    type = ElementIntegralVariablePostprocessor
    variable = p
    execute_on = linear
    block = 'matrix'
  []
  [max_vel_y]
    type = ElementExtremeValue
    variable = vel_y_fluid
    block = 'matrix'
    value_type = max
  []
  [min_vel_y]
    type = ElementExtremeValue
    variable = vel_y_fluid
    block = 'matrix'
    value_type = min
  []
[]

[Outputs]
  hide = 'pressure_integral lambda'
  [csv]
    type = CSV
    execute_on = 'final'
  []
[]
