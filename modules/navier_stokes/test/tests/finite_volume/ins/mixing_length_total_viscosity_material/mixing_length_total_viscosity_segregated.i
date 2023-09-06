von_karman_const = 0.41

H = 1 #halfwidth of the channel
L = 150

Re = 13700

rho = 1
bulk_u = 1
mu = ${fparse rho * bulk_u * 2 * H / Re}

advected_interp_method='upwind'
velocity_interp_method='rc'

pressure_tag = "pressure_grad"

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolatorSegregated
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${L}'
    dy = '0.667 0.333'
    ix = '200'
    iy = '10  1'
  []
[]

[Problem]
  nl_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
  error_on_jacobian_nonzero_reallocation = true
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
    nl_sys = u_system
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
    nl_sys = v_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    nl_sys = pressure_system
    initial_condition = 0.2
    two_term_boundary_expansion = false
  []
[]

[AuxVariables]
  [mixing_length]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []
  [u_viscosity_rans]
    type = INSFVMixingLengthReynoldsStress
    variable = vel_x
    rho = ${rho}
    mixing_length = mixing_length
    momentum_component = 'x'
    u = vel_x
    v = vel_y
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []
  [v_viscosity_rans]
    type = INSFVMixingLengthReynoldsStress
    variable = vel_y
    rho = ${rho}
    mixing_length = mixing_length
    momentum_component = 'y'
    u = vel_x
    v = vel_y
  []

  [p_diffusion]
    type = FVDiffusion
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

[AuxKernels]
  [mixing_len]
    type = WallDistanceMixingLengthAux
    walls = 'top'
    variable = mixing_length
    execute_on = 'initial'
    von_karman_const = ${von_karman_const}
    delta = 0.5
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    function = '1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    function = '0'
  []
  [wall-u]
    type = INSFVWallFunctionBC
    variable = vel_x
    boundary = 'top'
    u = vel_x
    v = vel_y
    mu = ${mu}
    rho = ${rho}
    momentum_component = x
  []
  [wall-v]
    type = INSFVWallFunctionBC
    variable = vel_y
    boundary = 'top'
    u = vel_x
    v = vel_y
    mu = ${mu}
    rho = ${rho}
    momentum_component = y
  []
  [sym-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = total_viscosity
    momentum_component = x
  []
  [sym-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = total_viscosity
    momentum_component = y
  []
  [symmetry_pressure]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom'
    variable = pressure
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = '0'
  []
[]

[Materials]
  [total_viscosity]
    type = MixingLengthTurbulentViscosityMaterial
    u = 'vel_x'                             #computes total viscosity = mu_t + mu
    v = 'vel_y'                             #property is called total_viscosity
    mixing_length = mixing_length
    mu = ${mu}
    rho = ${rho}
  []
[]

[Executioner]
  type = SIMPLE
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.2
  num_iterations = 30
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  # momentum_petsc_options = '-ksp_monitor'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  # pressure_petsc_options = '-ksp_monitor'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'

  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  momentum_l_max_its = 30
  pressure_l_max_its = 30
  momentum_l_tol = 0.0
  pressure_l_tol = 0.0
  print_fields = false
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'final'
  []
[]
