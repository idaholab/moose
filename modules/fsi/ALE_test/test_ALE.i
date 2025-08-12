
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = FileMesh
  file = tmesh_HR.msh
[]

[Variables]
  [disp_x]
    order = SECOND
    family = LAGRANGE
  []
  [disp_y]
    order = SECOND
    family = LAGRANGE
  []
  [vel]
    family = LAGRANGE_VEC
    block = 'matrix'
  []
  [p]
    block = 'matrix'
  []
  [vel_x_solid]
    block = 'inclusion'
  []
  [vel_y_solid]
    block = 'inclusion'
  []
[]

[AuxVariables]

  [accel_x]
    block = 'inclusion'
  []
  [accel_y]
    block = 'inclusion'
  []
[]

[AuxKernels]

  [accel_x] # Calculates and stores acceleration at the end of time step
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x_solid
    beta = 0.25
    execute_on = timestep_end
    block = 'inclusion'
  []
  [accel_y] # Calculates and stores acceleration at the end of time step
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y_solid
    beta = 0.25
    execute_on = timestep_end
    block = 'inclusion'
  []
[]

[Kernels]
  [mat_disp_x]
    type = Diffusion
    variable = disp_x
    block = 'matrix'
    use_displaced_mesh = false
  []
  [mat_disp_y]
    type = Diffusion
    variable = disp_y
    block = 'matrix'
    use_displaced_mesh = false
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

  # Solid velocity and acceleration
  [accel_tensor_x]
    type = CoupledTimeDerivative
    variable = disp_x
    v = vel_x_solid
    block = 'inclusion'
    use_displaced_mesh = false
  []
  [accel_tensor_y]
    type = CoupledTimeDerivative
    variable = disp_y
    v = vel_y_solid
    block = 'inclusion'
    use_displaced_mesh = false
  []
  [vxs_time_derivative_term]
    type = CoupledTimeDerivative
    variable = vel_x_solid
    v = disp_x
    block = 'inclusion'
    use_displaced_mesh = false
  []
  [vys_time_derivative_term]
    type = CoupledTimeDerivative
    variable = vel_y_solid
    v = disp_y
    block = 'inclusion'
    use_displaced_mesh = false
  []

  ## Solid Mech Dynamics
  [DynamicSolidMechanics] # zeta*K*vel + K * disp
    displacements = 'disp_x disp_y'
    stiffness_damping_coefficient = 0.000025
    block = 'inclusion'
  []

  [inertia_x] # M*accel + eta*M*vel
    type = InertialForce
    variable = disp_x
    velocity = vel_x_solid
    acceleration = accel_x
    beta = 0.25 # Newmark time integration
    gamma = 0.5 # Newmark time integration
    eta = 19.63
    block = 'inclusion'
  []

  [inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_x_solid
    acceleration = accel_y
    beta = 0.25
    gamma = 0.5
    eta = 19.63
    block = 'inclusion'
  []
[]

[InterfaceKernels]
  [penalty]
    type = ADPenaltyVelocityContinuity
    variable = vel
    fluid_velocity = vel
    displacements = 'disp_x disp_y'
    solid_velocities = 'vel_x_solid vel_y_solid'
    boundary = 'inclusion_boundary'
    penalty = 1e6
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
    youngs_modulus = 1e8
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
  automatic_scaling = true
  solve_type = 'NEWTON'
  end_time = 100.0
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 5
    dt = 0.005
    growth_factor = 1.5
    cutback_factor = 0.9
  []
[]

[Outputs]
  exodus = true
[]
