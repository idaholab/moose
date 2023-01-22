[GlobalParams]
  gravity = '0 0 0'
  integrate_p_by_parts = true
  laplace = true
  convective_term = true
  transient_term = true
  pspg = true
  displacements = 'disp_x disp_y'
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
  [./vel_x]
    block = 0
  [../]
  [./vel_y]
    block = 0
  [../]
  [./p]
    block = 0
  [../]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./vel_x_solid]
    block = 1
  [../]
  [./vel_y_solid]
    block = 1
  [../]
[]

[Kernels]
  [./vel_x_time]
    type = INSMomentumTimeDerivative
    variable = vel_x
    block = 0
    use_displaced_mesh = true
  [../]
  [./vel_y_time]
    type = INSMomentumTimeDerivative
    variable = vel_y
    block = 0
    use_displaced_mesh = true
  [../]
  [./mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
    pressure = p
    block = 0
    use_displaced_mesh = true
  [../]
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    u = vel_x
    v = vel_y
    pressure = p
    component = 0
    block = 0
    use_displaced_mesh = true
  [../]
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    u = vel_x
    v = vel_y
    pressure = p
    component = 1
    block = 0
    use_displaced_mesh = true
  [../]
  [./vel_x_mesh]
    type = ConvectedMesh
    disp_x = disp_x
    disp_y = disp_y
    variable = vel_x
    block = 0
    use_displaced_mesh = true
  [../]
  [./vel_y_mesh]
    type = ConvectedMesh
    disp_x = disp_x
    disp_y = disp_y
    variable = vel_y
    block = 0
    use_displaced_mesh = true
  [../]
  [./disp_x_fluid]
    type = Diffusion
    variable = disp_x
    block = 0
  [../]
  [./disp_y_fluid]
    type = Diffusion
    variable = disp_y
    block = 0
  [../]
  [./accel_tensor_x]
    type = CoupledTimeDerivative
    variable = disp_x
    v = vel_x_solid
    block = 1
  [../]
  [./accel_tensor_y]
    type = CoupledTimeDerivative
    variable = disp_y
    v = vel_y_solid
    block = 1
  [../]
  [./vxs_time_derivative_term]
    type = CoupledTimeDerivative
    variable = vel_x_solid
    v = disp_x
    block = 1
  [../]
  [./vys_time_derivative_term]
    type = CoupledTimeDerivative
    variable = vel_y_solid
    v = disp_y
    block = 1
  [../]
  [./source_vxs]
    type = MatReaction
    variable = vel_x_solid
    block = 1
    mob_name = 1
  [../]
  [./source_vys]
    type = MatReaction
    variable = vel_y_solid
    block = 1
    mob_name = 1
  [../]
[]

[InterfaceKernels]
  [./penalty_interface_x]
    type = CoupledPenaltyInterfaceDiffusion
    variable = vel_x
    neighbor_var = disp_x
    secondary_coupled_var = vel_x_solid
    boundary = master0_interface
    penalty = 1e6
  [../]
  [./penalty_interface_y]
    type = CoupledPenaltyInterfaceDiffusion
    variable = vel_y
    neighbor_var = disp_y
    secondary_coupled_var = vel_y_solid
    boundary = master0_interface
    penalty = 1e6
  [../]
[]

[Modules/TensorMechanics/Master]
  [./solid_domain]
    strain = SMALL
    incremental = false
    # generate_output = 'strain_xx strain_yy strain_zz' ## Not at all necessary, but nice
    block = '1'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e2
    poissons_ratio = 0.3
    block = '1'
  [../]
  [./small_stress]
    type = ComputeLinearElasticStress
    block = 1
  [../]
  [./const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '1  1'
  [../]
[]

[BCs]
  [./fluid_x_no_slip]
    type = DirichletBC
    variable = vel_x
    boundary = 'bottom'
    value = 0.0
  [../]
  [./fluid_y_no_slip]
    type = DirichletBC
    variable = vel_y
    boundary = 'bottom left_to_0'
    value = 0.0
  [../]
  [./x_inlet]
    type = FunctionDirichletBC
    variable = vel_x
    boundary = 'left_to_0'
    function = 'inlet_func'
  [../]
  [./no_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'bottom top left_to_1 right_to_1 left_to_0 right_to_0'
    value = 0
  [../]
  [./no_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom top left_to_1 right_to_1 left_to_0 right_to_0'
    value = 0
  [../]
  [./solid_x_no_slip]
    type = DirichletBC
    variable = vel_x_solid
    boundary = 'top left_to_1 right_to_1'
    value = 0.0
  [../]
  [./solid_y_no_slip]
    type = DirichletBC
    variable = vel_y_solid
    boundary = 'top left_to_1 right_to_1'
    value = 0.0
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  # num_steps = 60
  dt = 0.1
  dtmin = 0.1
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = none
[]

[Outputs]
  [./out]
    type = Exodus
  [../]
[]

[Functions]
  [./inlet_func]
    type = ParsedFunction
    expression = '(-16 * (y - 0.25)^2 + 1) * (1 + cos(t))'
  [../]
[]
