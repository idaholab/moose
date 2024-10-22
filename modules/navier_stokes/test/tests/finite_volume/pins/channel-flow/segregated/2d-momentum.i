mu = 1.1
rho = 1.1

pressure_tag = "pressure_grad"

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = 0
    ymax = 1
    nx = 40
    ny = 6
  []
[]

[GlobalParams]
  advected_interp_method = 'average'
  velocity_interp_method = 'rc'
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolatorSegregated
    u = superficial_vel_x
    v = superficial_vel_y
    pressure = pressure
    porosity = porosity
  []
[]

[Problem]
  nl_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[Variables]
  [superficial_vel_x]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1
    solver_sys = u_system
    two_term_boundary_expansion = false
  []
  [superficial_vel_y]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1e-6
    solver_sys = v_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    two_term_boundary_expansion = false
    solver_sys = pressure_system
  []
[]

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.5
  []
[]

[FVKernels]
  inactive = "u_friction v_friction"
  [u_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_vel_x
    rho = ${rho}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_x
    mu = ${mu}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_x
    momentum_component = 'x'
    pressure = pressure
    porosity = porosity
    extra_vector_tags = ${pressure_tag}
  []
  [u_friction]
    type = PINSFVMomentumFriction
    variable = superficial_vel_x
    momentum_component = 'y'
    Darcy_name = 'Darcy_coefficient'
    Forchheimer_name = 'Forchheimer_coefficient'
    rho = ${rho}
    speed = speed
    mu = ${mu}
  []

  [v_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_vel_y
    rho = ${rho}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_y
    mu = ${mu}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_y
    momentum_component = 'y'
    pressure = pressure
    porosity = porosity
    extra_vector_tags = ${pressure_tag}
  []
  [v_friction]
    type = PINSFVMomentumFriction
    variable = superficial_vel_y
    momentum_component = 'y'
    Darcy_name = 'Darcy_coefficient'
    Forchheimer_name = 'Forchheimer_coefficient'
    rho = ${rho}
    speed = speed
    mu = ${mu}
  []

  [p_diffusion]
    type = FVAnisotropicDiffusion
    variable = pressure
    coeff = "Ainv"
    coeff_interp_method = 'average'
  []
  [p_source]
    type = FVDivergence
    variable = pressure
    vector_field = "HbyA"
    force_boundary_execution = true
  []
[]

[FVBCs]
  inactive = 'slip-u slip-v'
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = superficial_vel_x
    function = '1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = superficial_vel_y
    function = 0
  []
  [no-slip-u]
    type = INSFVNoSlipWallBC
    boundary = 'top'
    variable = superficial_vel_x
    function = 0
  []
  [no-slip-v]
    type = INSFVNoSlipWallBC
    boundary = 'top'
    variable = superficial_vel_y
    function = 0
  []
  [symmetry-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = superficial_vel_x
    u = superficial_vel_x
    v = superficial_vel_y
    mu = ${mu}
    momentum_component = 'x'
  []
  [symmetry-v]
    type = PINSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = superficial_vel_y
    u = superficial_vel_x
    v = superficial_vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [symmetry-p]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom'
    variable = pressure
  []
  [outlet-p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0.4
  []
  ### Are disabled by default but we switch it on for certain tests ###
  [slip-u]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top'
    variable = superficial_vel_x
    momentum_component = 'x'
  []
  [slip-v]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top'
    variable = superficial_vel_y
    momentum_component = 'y'
  []
  #####################################################################
[]

[FunctorMaterials]
  [darcy]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Darcy_coefficient Forchheimer_coefficient'
    prop_values = '0.01 0.02 0.03 0.01 0.02 0.03'
  []
  [speed]
    type = PINSFVSpeedFunctorMaterial
    superficial_vel_x = superficial_vel_x
    superficial_vel_y = superficial_vel_y
    porosity = porosity
  []
[]

[Executioner]
  type = SIMPLENonlinearAssembly
  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.85
  pressure_variable_relaxation = 0.45
  num_iterations = 150
  pressure_absolute_tolerance = 1e-13
  momentum_absolute_tolerance = 1e-13
  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
[]
