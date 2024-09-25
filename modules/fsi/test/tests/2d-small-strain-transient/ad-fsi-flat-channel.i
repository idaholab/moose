[GlobalParams]
  displacements = 'disp_x disp_y'
  order = FIRST
  preset = false
  use_displaced_mesh = true
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 3.0
    ymin = 0
    ymax = 1.0
    nx = 10
    ny = 15
    elem_type = QUAD4
  []
  [subdomain1]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.0 0.5 0'
    block_id = 1
    top_right = '3.0 1.0 0'
    input = gmg
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'master0_interface'
    input = subdomain1
  []
  [break_boundary]
    type = BreakBoundaryOnSubdomainGenerator
    input = interface
  []
[]

[Variables]
  [vel]
    block = 0
    family = LAGRANGE_VEC
  []
  [p]
    block = 0
    order = FIRST
  []
  [disp_x]
  []
  [disp_y]
  []
  [vel_x_solid]
    block = 1
  []
  [vel_y_solid]
    block = 1
  []
[]

[Kernels]
  [mass]
    type = INSADMass
    variable = p
    block = 0
  []
  [mass_pspg]
    type = INSADMassPSPG
    variable = p
    block = 0
  []
  [momentum_time]
    type = INSADMomentumTimeDerivative
    variable = vel
    block = 0
  []
  [momentum_convection]
    type = INSADMomentumAdvection
    variable = vel
    block = 0
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = vel
    block = 0
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = vel
    pressure = p
    integrate_p_by_parts = true
    block = 0
  []
  [momentum_supg]
    type = INSADMomentumSUPG
    variable = vel
    material_velocity = relative_velocity
    block = 0
  []
  [momentum_mesh]
    type = INSADMomentumMeshAdvection
    variable = vel
    disp_x = 'disp_x'
    disp_y = 'disp_y'
    block = 0
  []
  [disp_x_fluid]
    type = Diffusion
    variable = disp_x
    block = 0
    use_displaced_mesh = false
  []
  [disp_y_fluid]
    type = Diffusion
    variable = disp_y
    block = 0
    use_displaced_mesh = false
  []
  [accel_tensor_x]
    type = CoupledTimeDerivative
    variable = disp_x
    v = vel_x_solid
    block = 1
    use_displaced_mesh = false
  []
  [accel_tensor_y]
    type = CoupledTimeDerivative
    variable = disp_y
    v = vel_y_solid
    block = 1
    use_displaced_mesh = false
  []
  [vxs_time_derivative_term]
    type = CoupledTimeDerivative
    variable = vel_x_solid
    v = disp_x
    block = 1
    use_displaced_mesh = false
  []
  [vys_time_derivative_term]
    type = CoupledTimeDerivative
    variable = vel_y_solid
    v = disp_y
    block = 1
    use_displaced_mesh = false
  []
  [source_vxs]
    type = MatReaction
    variable = vel_x_solid
    block = 1
    reaction_rate = 1
    use_displaced_mesh = false
  []
  [source_vys]
    type = MatReaction
    variable = vel_y_solid
    block = 1
    reaction_rate = 1
    use_displaced_mesh = false
  []
[]

[InterfaceKernels]
  [penalty]
    type = ADPenaltyVelocityContinuity
    variable = vel
    fluid_velocity = vel
    displacements = 'disp_x disp_y'
    solid_velocities = 'vel_x_solid vel_y_solid'
    boundary = master0_interface
    penalty = 1e6
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [solid_domain]
    strain = SMALL
    incremental = false
    # generate_output = 'strain_xx strain_yy strain_zz' ## Not at all necessary, but nice
    block = '1'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e2
    poissons_ratio = 0.3
    block = '1'
    use_displaced_mesh = false
  []
  [small_stress]
    type = ComputeLinearElasticStress
    block = 1
  []
  [const]
    type = ADGenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '1  1'
  []
  [ins_mat]
    type = INSADTauMaterial
    velocity = vel
    pressure = p
    block = 0
  []
[]

[BCs]
  [fluid_bottom]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = 'bottom'
    function_x = 0
    function_y = 0
  []
  [fluid_left]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = 'left_to_0'
    function_x = 'inlet_func'
    function_y = 0
    # The displacements actually affect the result of the function evaluation so in order to eliminate the impact
    # on the Jacobian we set 'use_displaced_mesh = false' here
    use_displaced_mesh = false
  []
  [no_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'bottom top left_to_1 right_to_1 left_to_0 right_to_0'
    value = 0
  []
  [no_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom top left_to_1 right_to_1 left_to_0 right_to_0'
    value = 0
  []
  [solid_x_no_slip]
    type = DirichletBC
    variable = vel_x_solid
    boundary = 'top left_to_1 right_to_1'
    value = 0.0
  []
  [solid_y_no_slip]
    type = DirichletBC
    variable = vel_y_solid
    boundary = 'top left_to_1 right_to_1'
    value = 0.0
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  # num_steps = 60
  dt = 0.1
  dtmin = 0.1
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = none
  nl_rel_tol = 1e-50
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]

[Functions]
  [inlet_func]
    type = ParsedFunction
    expression = '(-16 * (y - 0.25)^2 + 1) * (1 + cos(t))'
  []
[]
