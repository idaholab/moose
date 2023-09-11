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
    nx = 20
    ny = 10
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
  error_on_jacobian_nonzero_reallocation = true
[]

[Variables]
  [superficial_vel_x]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1
    nl_sys = u_system
    two_term_boundary_expansion = false
  []
  [superficial_vel_y]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1e-6
    nl_sys = v_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    two_term_boundary_expansion = false
    nl_sys = pressure_system
  []
[]

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.5
  []
[]

[FVKernels]
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

  # Possible outlet boundary conditions
  [outlet-p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0.4
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-10
  pressure_l_abs_tol = 1e-10
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.85
  pressure_variable_relaxation = 0.4
  num_iterations = 60
  pressure_absolute_tolerance = 1e-9
  momentum_absolute_tolerance = 1e-9
  print_fields = false
[]

[Outputs]
  exodus = true
[]
