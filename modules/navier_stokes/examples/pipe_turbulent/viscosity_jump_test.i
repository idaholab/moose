# Geometry
D = 0.02
total_len = '${fparse 40 * D}'
L = '${fparse 20 * D}'
nx = 50
ny = 10

# Crafted wall function
Re = 20000
rho = 1000.0
bulk_u = 1.0
mu = 1e-3
# mu = '${fparse bulk_u * rho * D / Re}'

# Crafted wall function
f = '${fparse 0.316 * Re^(-0.25)}'
ref_delta_P = '${fparse f * L / D * rho * bulk_u^2 / 2}'
tau_wall = '${fparse ref_delta_P / (pi * D * L)}'
u_tau = '${fparse sqrt(tau_wall / rho)}'
y_dist_wall = '${fparse D/ny/2}'
mu_wall = '${fparse rho * pow(u_tau,2) * y_dist_wall / bulk_u}'

# Crafted bulk viscosity
turbulent_intensity = 0.01 #'${fparse 0.16 * pow(Re, -1.0/8.0)}'
C_mu = 0.09
mixing_length = '${fparse D * 0.07}'
k_bulk = '${fparse 3/2 * pow(bulk_u*turbulent_intensity, 2)}'
eps_bulk = '${fparse pow(C_mu, 0.75) * pow(k_bulk, 1.5) / mixing_length}'
mu_bulk = '${fparse rho * C_mu * pow(k_bulk, 2) / eps_bulk}'

sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92

diff = 10.0
advected_interp_method = 'upwind'
velocity_interp_method = 'average'

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = ${advected_interp_method}
  velocity_interp_method = ${velocity_interp_method}
  two_term_boundary_expansion = true
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Mesh]
  # coord_type = 'RZ'
  # rz_coord_axis = 'X'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${L}
    ymin = '${fparse -D/2}'
    ymax = '${fparse D/2}'
    nx = ${nx}
    ny = ${ny}
    bias_y = 1.0
    bias_x = 1.0
  []
  # [rename1]
  #   type = RenameBoundaryGenerator
  #   input = gen
  #   old_boundary = 'left'
  #   new_boundary = 'inlet'
  # []
  # [rename2]
  #   type = RenameBoundaryGenerator
  #   input = rename1
  #   old_boundary = 'right'
  #   new_boundary = 'outlet'
  # []
  # [rename3]
  #   type = RenameBoundaryGenerator
  #   input = rename2
  #   old_boundary = 'bottom'
  #   new_boundary = 'symmetry'
  # []
  # [rename4]
  #   type = RenameBoundaryGenerator
  #   input = rename3
  #   old_boundary = 'top'
  #   new_boundary = 'wall'
  # []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = ${bulk_u}
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0
  []
  [pressure]
    type = INSFVPressureVariable
  []
  [TKE]
    type = INSFVEnergyVariable
    initial_condition = ${k_bulk}
    #scaling = '${fparse 1/k_bulk}'
  []
  [TKED]
    type = INSFVEnergyVariable
    initial_condition = ${eps_bulk}
    #scaling = '${fparse 1/eps_bulk}'
  []
[]

[FVKernels]

  inactive = 'u_time v_time TKE_time TKED_time'

  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = ${rho}
  []

  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = 'mu_t'
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []

  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = 'mu_t'
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []

  [TKE_time]
    type = INSFVEnergyTimeDerivative
    variable = TKE
    cp = 1
    rho = ${rho}
  []
  [TKE_advection]
    type = INSFVTurbulentAdvection
    variable = TKE
    rho = ${rho}
    walls = 'bottom top'
  []
  [TKE_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKE
    coeff = 'mu_t'
    scaling_coef = ${sigma_k}
    walls = 'bottom top'
  []
  [TKE_source_sink]
    type = INSFVTKESourceSink
    variable = TKE
    u = vel_x
    v = vel_y
    epsilon = TKED
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    linearized_model = true
    rf = 1.0
    walls = 'bottom top'
    non_equilibrium_treatement = false
  []

  [TKED_time]
    type = INSFVEnergyTimeDerivative
    variable = TKED
    cp = 1
    rho = ${rho}
  []
  [TKED_advection]
    type = INSFVTurbulentAdvection
    variable = TKED
    rho = ${rho}
    walls = 'bottom top'
  []
  [TKED_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKED
    coeff = 'mu_t'
    scaling_coef = ${sigma_eps}
    walls = 'bottom top'
  []
  [TKED_source_sink]
    type = INSFVTKEDSourceSink
    variable = TKED
    u = vel_x
    v = vel_y
    k = TKE
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    C1_eps = ${C1_eps}
    C2_eps = ${C2_eps}
    rf = 1.0
    walls = 'bottom top'
    # non_equilibrium_treatement = false
  []
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [mu_t]
    type = MooseVariableFVReal
    initial_condition = '${fparse mu_bulk}'
  []
  [mu_t_computed]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
  []
  [compute_mu_t]
    type = kEpsilonViscosityAux
    variable = mu_t
    C_mu = ${C_mu}
    k = TKE
    epsilon = TKED
    mu = ${mu}
    rho = ${rho}
    u = vel_x
    v = vel_y
    wall_treatement = false
    walls = 'top bottom'
    non_equilibrium_treatement = false
    rf = 1.0
    execute_on = 'TIMESTEP_END'
  []
  [populate_mu_t_computed]
    type = ADFunctorElementalAux
    variable = 'mu_t_computed'
    functor = 'mu_t_imposed'
  []
  [populate_mu_t]
    type = ADFunctorElementalAux
    variable = 'mu_t'
    functor = 'mu_t_imposed'
    execute_on = 'INITIAL'
  []
[]

[Problem]
  previous_nl_solution_required = true
[]

[Functions]
  # Not working
  [viscous_jump]
    type = ADParsedFunction
    expression = 'if((abs(y) > (D)*(ny/2 -1)/(ny/2)), mu_wall, mu_bulk)'
    symbol_names = 'D ny mu_wall mu_bulk'
    symbol_values = '${D} ${ny} ${mu_wall} ${mu_bulk}'
  []
[]

[Materials]
  [viscosity]
    type = ADGenericFunctorMaterial
    prop_names = 'mu_t_imposed'
    prop_values = 'viscous_jump'
  []
  # [mu_t_material]
  #   type = INSFVkEpsilonViscosityMaterial
  #   k = TKE
  #   epsilon = TKED
  #   rho = ${rho}
  # []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    function = '${bulk_u}'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    function = 0
  []
  [walls-u]
    type = FVDirichletBC
    boundary = 'top bottom'
    variable = vel_x
    value = 0
  []
  [walls-v]
    type = FVDirichletBC
    boundary = 'top bottom'
    variable = vel_y
    value = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0
  []
  [inlet_TKE]
    type = FVDirichletBC
    boundary = 'left'
    variable = TKE
    value = ${k_bulk}
  []
  [inlet_TKED]
    type = FVDirichletBC
    boundary = 'left'
    variable = TKED
    value = ${eps_bulk}
  []
  [walls_mu_t]
    type = INSFVTurbulentViscosityWallFunction
    boundary = 'top bottom'
    variable = mu_t
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    mu_t = mu_t
    k = TKE
  []
  # [wall_TKED]
  #   type = INSFVTKEDWallFunction
  #   boundary = 'top bottom'
  #   variable = TKED
  #   k = TKE
  #   rho = ${rho}
  #   mu = ${mu}
  #   mu_t = mu_t
  #   u = vel_x
  #   v = vel_y
  # []
  # [outlet_TKE]
  #   type = INSFVMassAdvectionOutflowBC
  #   variable = TKE
  #   boundary = 'right'
  #   u = vel_x
  #   v = vel_y
  #   rho = ${rho}
  # []
  # [outlet_TKED]
  #   type = INSFVMassAdvectionOutflowBC
  #   variable = TKED
  #   boundary = 'right'
  #   u = vel_x
  #   v = vel_y
  #   rho = ${rho}
  # []
  # [outlet_u]
  #   type = INSFVMomentumAdvectionOutflowBC
  #   variable = u
  #   boundary = 'top_to_0'
  #   u = u
  #   v = v
  #   momentum_component = 'x'
  #   rho = ${rho}
  # []
  # [outlet_v]
  #   type = INSFVMomentumAdvectionOutflowBC
  #   variable = v
  #   boundary = 'top_to_0'
  #   u = u
  #   v = v
  #   momentum_component = 'y'
  #   rho = ${rho}
  # []
[]

[Debug]
  show_var_residual_norms = true
[]

[Executioner]
  type = Transient
  end_time = 100
  dt = 1
  # [TimeStepper]
  #   type = IterationAdaptiveDT
  #   dt = 0.001
  #   iteration_window = 2
  #   optimal_iterations = 10
  #   growth_factor = 1.2
  #   cutback_factor = 0.8
  # []
  steady_state_detection = true
  steady_state_tolerance = 1e-6
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -snes_linesearch_damping'
  petsc_options_value = 'lu        NONZERO               0.9'
  nl_abs_tol = 1e-8
  nl_max_its = 2000
  line_search = none
[]

[Outputs]
  exodus = true
[]
