# This file simulates flow of fluid in a porous elbow for the purpose of verifying
# correct implementation of the various different solution variable sets. This input
# tests correct implementation of the primitive superficial variable set. Flow enters on the top
# and exits on the right. Because the purpose is only to test the equivalence of
# different equation sets, no solid energy equation is included.

porosity_left = 0.4
porosity_right = 0.6
pebble_diameter = 0.06

mu = 1.81e-5 # This has been increased to avoid refining the mesh
M = 28.97e-3
R = 8.3144598

# inlet mass flowrate, kg/s
mdot = -10.0

# inlet mass flux (superficial)
mflux_in_superficial = ${fparse mdot / (pi * 0.5 * 0.5)}

# inlet mass flux (interstitial)
mflux_in_interstitial = ${fparse mflux_in_superficial / porosity_left}

p_initial = 201325.0
T_initial = 300.0
rho_initial = ${fparse p_initial / T_initial * M / R}

vel_y_initial = ${fparse mflux_in_interstitial / rho_initial}
vel_x_initial = 0.0

superficial_vel_y_initial = ${fparse mflux_in_superficial / rho_initial}
superficial_vel_x_initial = 1e-12

# Computation parameters
velocity_interp_method = 'rc'
advected_interp_method = 'upwind'

# ==============================================================================
# GEOMETRY AND MESH
# ==============================================================================

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'ergun_in.e'
  []
  coord_type = RZ
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = superficial_vel_x
    v = superficial_vel_y
    pressure = pressure
    porosity = porosity
  []
[]

[GlobalParams]
  porosity = porosity
  pebble_diameter = ${pebble_diameter}
  fp = fp

  # rho for the kernels. Must match fluid property!
  rho = ${rho_initial}

  fv = true
  velocity_interp_method = ${velocity_interp_method}
  advected_interp_method = ${advected_interp_method}

  # behavior at time of test creation
  two_term_boundary_expansion = false
  rhie_chow_user_object = 'rc'
[]

# ==============================================================================
# VARIABLES AND KERNELS
# ==============================================================================

[Variables]
  [pressure]
    type = INSFVPressureVariable
    initial_condition = ${p_initial}
  []
  [superficial_vel_x]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = ${superficial_vel_x_initial}
  []
  [superficial_vel_y]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = ${superficial_vel_y_initial}
  []
[]

[FVKernels]
  # Mass Equation.
  [mass]
    type = PINSFVMassAdvection
    variable = 'pressure'
  []

  # Momentum x component equation.
  [vel_x_time]
    type = PINSFVMomentumTimeDerivative
    variable = 'superficial_vel_x'
    momentum_component = 'x'
  []
  [vel_x_advection]
    type = PINSFVMomentumAdvection
    variable = 'superficial_vel_x'
    momentum_component = 'x'
  []
  [vel_x_viscosity]
    type = PINSFVMomentumDiffusion
    variable = 'superficial_vel_x'
    momentum_component = 'x'
    mu = 'mu'
  []
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = 'superficial_vel_x'
    pressure = pressure
    momentum_component = 'x'
  []
  [u_friction]
    type = PINSFVMomentumFriction
    variable = 'superficial_vel_x'
    Darcy_name = 'Darcy_coefficient'
    Forchheimer_name = 'Forchheimer_coefficient'
    momentum_component = 'x'
    speed = speed
    mu = 'mu'
  []

  # Momentum y component equation.
  [vel_y_time]
    type = PINSFVMomentumTimeDerivative
    variable = 'superficial_vel_y'
    momentum_component = 'y'
  []
  [vel_y_advection]
    type = PINSFVMomentumAdvection
    variable = 'superficial_vel_y'
    momentum_component = 'y'
  []
  [vel_y_viscosity]
    type = PINSFVMomentumDiffusion
    variable = 'superficial_vel_y'
    momentum_component = 'y'
    mu = 'mu'
  []
  [v_pressure]
    type = PINSFVMomentumPressure
    variable = 'superficial_vel_y'
    pressure = pressure
    momentum_component = 'y'
  []
  [v_friction]
    type = PINSFVMomentumFriction
    variable = 'superficial_vel_y'
    Darcy_name = 'Darcy_coefficient'
    Forchheimer_name = 'Forchheimer_coefficient'
    momentum_component = 'y'
    mu = 'mu'
    speed = speed
  []
  [gravity]
    type = PINSFVMomentumGravity
    variable = 'superficial_vel_y'
    gravity = '0 -9.81 0'
    momentum_component = 'y'
  []
[]

# ==============================================================================
# AUXVARIABLES AND AUXKERNELS
# ==============================================================================

[AuxVariables]
  [T_fluid]
    initial_condition = ${T_initial}
    order = CONSTANT
    family = MONOMIAL
  []
  [vel_x]
    initial_condition = ${fparse vel_x_initial}
    order = CONSTANT
    family = MONOMIAL
  []
  [vel_y]
    initial_condition = ${fparse vel_y_initial}
    order = CONSTANT
    family = MONOMIAL
  []
  [porosity_out]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [vel_x]
    type = FunctorAux
    variable = vel_x
    functor = vel_x_mat
  []
  [vel_y]
    type = FunctorAux
    variable = vel_y
    functor = vel_y_mat
  []
  [porosity_out]
    type = FunctorAux
    variable = porosity_out
    functor = porosity
  []
[]

# ==============================================================================
# FLUID PROPERTIES, MATERIALS AND USER OBJECTS
# ==============================================================================

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    k = 0.0
    mu = ${mu}
    gamma = 1.4
    molar_mass = ${M}
  []
[]

[FunctorMaterials]
  [enthalpy]
    type = INSFVEnthalpyMaterial
    temperature = 'T_fluid'
  []
  [speed]
    type = PINSFVSpeedFunctorMaterial
    superficial_vel_x = 'superficial_vel_x'
    superficial_vel_y = 'superficial_vel_y'
    porosity = porosity
    vel_x = vel_x_mat
    vel_y = vel_y_mat
  []
  [kappa]
    type = FunctorKappaFluid
  []
  [const_Fdrags_mat]
    type = FunctorErgunDragCoefficients
    porosity = porosity
  []
  [fluidprops]
    type = GeneralFunctorFluidProps
    mu_rampdown = mu_func
    porosity = porosity
    characteristic_length = ${pebble_diameter}
    T_fluid = 'T_fluid'
    pressure = 'pressure'
    speed = 'speed'
  []
[]

d = 0.05

[Functions]
  [mu_func]
    type = PiecewiseLinear
    x = '1 3 5 10 15 20'
    y = '1e5 1e4 1e3 1e2 1e1 1'
  []
  [real_porosity_function]
    type = ParsedFunction
    expression = 'if (x < 0.6 - ${d}, ${porosity_left}, if (x > 0.6 + ${d}, ${porosity_right},
        (x-(0.6-${d}))/(2*${d})*(${porosity_right}-${porosity_left}) + ${porosity_left}))'
  []
  [porosity]
    type = ParsedFunction
    expression = 'if (x < 0.6 - ${d}, ${porosity_left}, if (x > 0.6 + ${d}, ${porosity_right},
        (x-(0.6-${d}))/(2*${d})*(${porosity_right}-${porosity_left}) + ${porosity_left}))'
  []
[]

# ==============================================================================
# BOUNDARY CONDITIONS
# ==============================================================================

[FVBCs]
  [outlet_p]
    type = INSFVOutletPressureBC
    variable = 'pressure'
    function = ${p_initial}
    boundary = 'right'
  []

  ## No or Free slip BC
  [free-slip-wall-x]
    type = INSFVNaturalFreeSlipBC
    boundary = 'bottom wall_1 wall_2 left'
    variable = superficial_vel_x
    momentum_component = 'x'
  []
  [free-slip-wall-y]
    type = INSFVNaturalFreeSlipBC
    boundary = 'bottom wall_1 wall_2 left'
    variable = superficial_vel_y
    momentum_component = 'y'
  []

  ## Symmetry
  [symmetry-x]
    type = PINSFVSymmetryVelocityBC
    boundary = 'left'
    variable = superficial_vel_x
    u = superficial_vel_x
    v = superficial_vel_y
    mu = 'mu'
    momentum_component = 'x'
  []
  [symmetry-y]
    type = PINSFVSymmetryVelocityBC
    boundary = 'left'
    variable = superficial_vel_y
    u = superficial_vel_x
    v = superficial_vel_y
    mu = 'mu'
    momentum_component = 'y'
  []
  [symmetry-p]
    type = INSFVSymmetryPressureBC
    boundary = 'left'
    variable = 'pressure'
  []

  ## inlet
  [inlet_vel_x]
    type = INSFVInletVelocityBC
    variable = 'superficial_vel_x'
    function = ${superficial_vel_x_initial}
    boundary = 'top'
  []
  [inlet_vel_y]
    type = INSFVInletVelocityBC
    variable = 'superficial_vel_y'
    function = ${superficial_vel_y_initial}
    boundary = 'top'
  []
[]
# ==============================================================================
# EXECUTION PARAMETERS
# ==============================================================================

[Executioner]
  type = Transient

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'asm      lu           NONZERO                   200'
  line_search = 'none'

  # Problem time parameters
  dtmin = 0.01
  dtmax = 2000
  end_time = 3000
  # must be the same as the fluid

  # Iterations parameters
  l_max_its = 50
  l_tol     = 1e-8
  nl_max_its = 25
  # nl_rel_tol = 5e-7
  nl_abs_tol = 2e-7

  # Automatic scaling
  automatic_scaling = true
  verbose = true

  [TimeStepper]
    type = IterationAdaptiveDT
    dt                 = 0.025
    cutback_factor     = 0.5
    growth_factor      = 2.0
  []

  # Steady state detection.
  steady_state_detection = true
  steady_state_tolerance = 1e-7
  steady_state_start_time = 400
[]

# ==============================================================================
# POSTPROCESSORS DEBUG AND OUTPUTS
# ==============================================================================

[Postprocessors]
  [mass_flow_in]
    type = VolumetricFlowRate
    boundary = 'top'
    vel_x = 'superficial_vel_x'
    vel_y = 'superficial_vel_y'
    advected_quantity = ${rho_initial}
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [mass_flow_out]
    type = VolumetricFlowRate
    boundary = 'right'
    vel_x = 'superficial_vel_x'
    vel_y = 'superficial_vel_y'
    advected_quantity = ${rho_initial}
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p_in]
    type = SideAverageValue
    variable = pressure
    boundary = 'top'
  []
  [dP]
    type = LinearCombinationPostprocessor
    pp_names = 'p_in'
    pp_coefs = '1.0'
    b = ${fparse -p_initial}
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
