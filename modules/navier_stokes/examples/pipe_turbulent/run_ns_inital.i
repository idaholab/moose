################################################################################
## Molten Salt Fast Reactor - Euratom EVOL + Rosatom MARS Design              ##
## Pronghorn input file to initialize velocity fields                         ##
## This runs a slow relaxation to steady state while ramping down the fluid   ##
## viscosity.                                                                 ##
################################################################################

# Material properties
rho = 4284 # density [kg / m^3]  (@1000K)
mu = 0.0166 # viscosity [Pa s]
# https://www.researchgate.net/publication/337161399_Development_of_a_control-\
# oriented_power_plant_simulator_for_the_molten_salt_fast_reactor/fulltext/5dc95c\
# 9da6fdcc57503eec39/Development-of-a-control-oriented-power-plant-simulator-for-the-molten-salt-fast-reactor.pdf
# Derived turbulent properties
von_karman_const = 0.41

# Mass flow rate tuning
friction = 4.0e3 # [kg / m^4]
pump_force = -20000. # [N / m^3]

advected_interp_method = 'upwind'
velocity_interp_method = 'rc'

sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

[GlobalParams]
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  advected_interp_method = ${advected_interp_method}
  velocity_interp_method = ${velocity_interp_method}
[]

################################################################################
# GEOMETRY
################################################################################

[Mesh]
  # uniform_refine = 1
  # [fmg]
  #   type = FileMeshGenerator
  #   file = 'msfr_rz_mesh.e'
  # []
  # [hx_top]
  #   type = ParsedGenerateSideset
  #   combinatorial_geometry = 'y > 0'
  #   included_subdomain_ids = '3'
  #   included_neighbor_ids = '1'
  #   fixed_normal = true
  #   normal = '0 1 0'
  #   new_sideset_name = 'hx_top'
  #   input = 'fmg'
  # []
  # [delete_unused]
  #   type = BlockDeletionGenerator
  #   input = 'hx_top'
  #   block = 'shield reflector'
  # []
  [fmg]
    type = FileMeshGenerator
    file = 'run_ns_inital_in.e'
  []
[]

[Problem]
  kernel_coverage_check = false
  coord_type = 'RZ'
  rz_coord_axis = Y
[]

################################################################################
# EQUATIONS: VARIABLES, KERNELS & BCS
################################################################################

[Modules]
  [NavierStokesFV]
    # General parameters
    compressibility = 'incompressible'
    gravity = '0 -9.81 0'

    # Material properties
    density = ${rho}
    dynamic_viscosity = 'mu_t'

    # Initial conditions
    initial_velocity = '1e-6 1e-6 0'
    initial_pressure = 1e5
    initial_temperature = 0.0

    # Boundary conditions
    wall_boundaries = 'shield_wall reflector_wall fluid_symmetry'
    momentum_wall_types = 'wallfunction wallfunction symmetry'

    # Pressure pin for incompressible flow
    pin_pressure = true
    pinned_pressure_type = average
    pinned_pressure_value = 1e5

    # Turbulence parameters
    #turbulence_handling = 'mixing-length'
    #von_karman_const = ${von_karman_const}
    #mixing_length_delta = 0.1
    #mixing_length_walls = 'shield_wall reflector_wall'
    #mixing_length_aux_execute_on = 'initial'

    # Heat exchanger friction
    friction_blocks = 'hx'
    friction_types = 'FORCHHEIMER'
    friction_coeffs = ${friction}

    # Numerical scheme
    mass_advection_interpolation = 'upwind'
    momentum_advection_interpolation = 'upwind'
  []
[]

[Variables]
  [TKE]
    type = INSFVEnergyVariable
    initial_condition = '${fparse 3/2 * 0.01}'
  []
  [TKED]
    type = INSFVEnergyVariable
    initial_condition = '${fparse pow(0.09, 0.75) * pow(3/2 * 0.01, 1.5) / 1.0}'
  []
[]

[FVKernels]
  [pump]
    type = INSFVBodyForce
    variable = vel_y
    functor = ${pump_force}
    block = 'pump'
    momentum_component = 'y'
  []

  #[TKE_time]
  #  type = INSFVEnergyTimeDerivative
  #  variable = TKE
  #  rho = ${rho}
  #  cp = 1.0
  #[]
  [TKE_advection]
    type = INSFVTurbulentAdvection
    variable = TKE
    rho = ${rho}
    walls = 'shield_wall reflector_wall'
  []
  [TKE_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKE
    coeff = 'mu_t'
    scaling_coef = ${sigma_k}
    walls = 'shield_wall reflector_wall'
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
    rf = 0.5
    walls = 'shield_wall reflector_wall'
  []

  #[TKED_time]
  #  type = INSFVEnergyTimeDerivative
  #  variable = TKED
  #  rho = ${rho}
  #  cp = 1.0
  #[]
  [TKED_advection]
    type = INSFVTurbulentAdvection
    variable = TKED
    rho = ${rho}
    walls = 'shield_wall reflector_wall'
  []
  [TKED_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKED
    coeff = 'mu_t'
    scaling_coef = ${sigma_eps}
    walls = 'shield_wall reflector_wall'
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
    rf = 0.5
    walls = 'shield_wall reflector_wall'
  []
[]

[AuxVariables]
  [mu_t]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
[]

[AuxKernels]
  [compute_mu_t]
    type = kEpsilonViscosityAux
    variable = mu_t
    C_mu = ${C_mu}
    k = TKE
    epsilon = TKED
    mu = 'mu'
    rho = ${rho}
    u = vel_x
    v = vel_y
    wall_treatement = false
    walls = 'shield_wall reflector_wall'
    non_equilibrium_treatement = false
    rf = 0.25
    execute_on = 'TIMESTEP_END'
  []
[]

[FVBCs]
  [walls_mu_t]
    type = INSFVTurbulentViscosityWallFunction
    boundary = 'shield_wall reflector_wall'
    variable = mu_t
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    mu_t = mu_t
    k = TKE
  []
[]

################################################################################
# MATERIALS
################################################################################

[Functions]
  [ad_rampdown_mu_func]
    type = ADParsedFunction
    value = mu*(100*exp(-3*t)+1)
    vars = 'mu'
    vals = ${mu}
  []
  # Duplicate definition to use in postprocessor,
  # we will convert types more in the future and avoid duplicates
  [rampdown_mu_func]
    type = ParsedFunction
    value = mu*(100*exp(-3*t)+1)
    vars = 'mu'
    vals = ${mu}
  []
[]

[Materials]
  [mu]
    type = ADGenericFunctorMaterial #defines mu artificially for numerical convergence
    prop_names = 'mu rho' #it converges to the real mu eventually.
    prop_values = 'ad_rampdown_mu_func ${rho}'
  []
  #[not_used]
  #  type = ADGenericFunctorMaterial
  #  prop_names = 'not_used'
  #  prop_values = 0
  #  block = 'shield reflector'
  #[]
[]

################################################################################
# EXECUTION / SOLVE
################################################################################

[Executioner]
  type = Transient

  # Time-stepping parameters
  start_time = 0.0
  end_time = 50.0

  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 20
    dt = 0.25
    timestep_limiting_postprocessor = 'dt_limit'
    cutback_factor = 0.5
    growth_factor = 2
  []

  # Solver parameters
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart -snes_linesearch_damping'
  petsc_options_value = 'lu        NONZERO               50                 0.9'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-6
  nl_max_its = 20 # fail early and try again with a shorter time step
  l_max_its = 50
  automatic_scaling = true
[]

################################################################################
# SIMULATION OUTPUTS
################################################################################

[Outputs]
  csv = true
  hide = 'dt_limit'
  exodus = true
  # Reduce base output
  print_linear_converged_reason = false
  print_linear_residuals = false
  print_nonlinear_converged_reason = false
[]

[Postprocessors]
  [max_v]
    type = ElementExtremeValue
    variable = vel_x
    value_type = max
    block = 'fuel pump hx'
  []
  [mu_value]
    type = FunctionValuePostprocessor
    function = rampdown_mu_func
  []
  [mdot]
    type = VolumetricFlowRate
    boundary = 'hx_top'
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = ${rho}
  []
  [dt_limit]
    type = Receiver
    default = 50
  []
[]

