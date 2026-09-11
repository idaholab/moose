# Two conducting slabs coupled to a conducting transparent gap by linear-FV
# conjugate heat transfer, with surface-to-surface radiation acting in parallel.
#
# The three regions are one-dimensional and have unit area:
#
#   1000 K | left solid | transparent conducting gap | right solid | 300 K
#          x=0         0.1                      0.2             0.3
#
# Analytic model:
#
#   q_total = k_s / L_s * (T_hot - T_left)
#           = k_f / L_f * (T_left - T_right) + q_rad
#           = k_s / L_s * (T_right - T_cold)
#
#   q_rad = sigma * (T_left^4 - T_right^4)
#           / (1 / eps_left + 1 / eps_right - 1)
#
# For equal solid resistances, define dT = T_left - T_right. The solid
# balances give T_left + T_right = T_hot + T_cold. Substitution into the gap
# balance produces one depressed cubic:
#
#   cubic_A*dT^3 + cubic_B*dT - cubic_C = 0.
#
# Its unique real root, and every analytical temperature and heat-flux
# reference below, are evaluated with HIT fparse expressions. Therefore,
# changing only k_fluid updates both the simulation and its analytical checks.
#
# LinearFVRobinCHTBC exchanges adjacent FV-cell temperatures. The numerical
# solution consequently approaches these continuum references at first order
# under refinement; no mesh-dependent reference values are used here.

T_hot = 1000.0
T_cold = 300.0

L_solid = 0.1
L_gap = 0.1

k_solid = 10.0
# Change this value to move between radiation-, mixed-, and
# conduction-dominated regimes. Examples: 0.1, 3.4180137348, and 50.0.
k_fluid = 50

eps_left = 0.8
eps_right = 0.6
sigma = 5.67037e-8

# Virtual Robin-Robin coupling coefficients. They affect fixed-point
# convergence, but not the converged interface solution.
h_f = 1.0
h_s = 1.0

# ---------------------------------------------------------------------------
# Continuum analytical solution. For dT = T_left - T_right,
#
#   A*dT^3 + B*dT - C = 0,
#   p = B/A, r = C/A,
#   dT = 2*sqrt(p/3)*sinh(asinh(3*r/(2*p)*sqrt(3/p))/3).
#
# This form avoids taking a fractional power of a negative number in Cardano's
# formula and has one real root because A, B, and C are positive.
temperature_sum = ${fparse T_hot+T_cold}
temperature_span = ${fparse T_hot-T_cold}
solid_resistance = ${fparse L_solid/k_solid}
gap_conductance = ${fparse k_fluid/L_gap}
emissivity_denominator = ${fparse 1/eps_left+1/eps_right-1}

cubic_A = ${fparse sigma*temperature_sum/(2*emissivity_denominator)}
cubic_B = ${fparse 1/(2*solid_resistance)+gap_conductance+sigma*temperature_sum^3/(2*emissivity_denominator)}
cubic_C = ${fparse temperature_span/(2*solid_resistance)}
cubic_p = ${fparse cubic_B/cubic_A}
cubic_r = ${fparse cubic_C/cubic_A}

interface_temperature_span = ${fparse 2*sqrt(cubic_p/3)*sinh(asinh(3*cubic_r/(2*cubic_p)*sqrt(3/cubic_p))/3)}

T_left_ref = ${fparse (temperature_sum+interface_temperature_span)/2}
T_right_ref = ${fparse (temperature_sum-interface_temperature_span)/2}
q_conduction_ref = ${fparse gap_conductance*interface_temperature_span}
q_radiation_ref = ${fparse sigma*(T_left_ref^4-T_right_ref^4)/emissivity_denominator}
q_total_ref = ${fparse q_conduction_ref+q_radiation_ref}

nx_solid = 200
nx_gap = 200

[Problem]
  kernel_coverage_check = false
  linear_sys_names = 'energy_system solid_energy_system u_system v_system p_system'
  previous_nl_solution_required = true
[]

[Mesh]
  type = MeshGeneratorMesh

  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${L_solid} ${L_gap} ${L_solid}'
    ix = '${nx_solid} ${nx_gap} ${nx_solid}'
    dy = '1.0'
    iy = '1'
    subdomain_id = '1 5 2'
  []

  # The sidesets are attached to the solid elements. This lets gray_lambert
  # sample T_solid on both radiating surfaces.
  [left_rad_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = cmg
    primary_block = 1
    paired_block = 5
    new_boundary = left_rad
  []

  [right_rad_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = left_rad_sideset
    primary_block = 2
    paired_block = 5
    new_boundary = right_rad
  []

  [rename_blocks]
    type = RenameBlockGenerator
    input = right_rad_sideset
    old_block = '1 5 2'
    new_block = 'left_solid fluid right_solid'
  []
[]

[Variables]
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = 600
    block = fluid
  []

  # A single solid variable/system spans the two disconnected slabs.
  [T_solid]
    type = MooseLinearVariableFVReal
    solver_sys = solid_energy_system
    initial_condition = 600
    block = 'left_solid right_solid'
  []

  # Dummy flow variables retain the SIMPLE setup. Their systems are disabled.
  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = 0
    block = fluid
  []

  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0
    block = fluid
  []

  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = p_system
    initial_condition = 0
    block = fluid
  []
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = 1
    p_diffusion_kernel = p_diffusion
    block = fluid
  []

  [view_factors_uo]
    type = SpecifiedViewFactor
    boundary = 'left_rad right_rad'
    view_factors = '0 1;
                    1 0'
    execute_on = INITIAL
  []

  [gray_lambert]
    type = ViewFactorObjectSurfaceRadiation
    boundary = 'left_rad right_rad'
    emissivity = '${eps_left} ${eps_right}'
    temperature = T_solid
    stefan_boltzmann_constant = ${sigma}
    view_factor_object_name = view_factors_uo
    execute_on = 'NONLINEAR TIMESTEP_END'
  []
[]

[LinearFVKernels]
  [fluid_conduction]
    type = LinearFVDiffusion
    variable = T_fluid
    diffusion_coeff = ${k_fluid}
    block = fluid
    use_nonorthogonal_correction = false
  []

  # CHTHandler expects one conduction kernel for the solid energy system.
  [solid_conduction]
    type = LinearFVDiffusion
    variable = T_solid
    diffusion_coeff = ${k_solid}
    block = 'left_solid right_solid'
    use_nonorthogonal_correction = false
  []

  # Required by RhieChowMassFlux although the pressure solve is disabled.
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    block = fluid
    use_nonorthogonal_correction = false
  []
[]

[LinearFVBCs]
  [hot_left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_solid
    boundary = left
    functor = ${T_hot}
  []

  [cold_right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_solid
    boundary = right
    functor = ${T_cold}
  []

  # Robin-Robin CHT at the left solid/gap interface.
  [fluid_left]
    type = LinearFVRobinCHTBC
    variable = T_fluid
    boundary = left_rad
    h = ${h_f}
    thermal_conductivity = ${k_fluid}
    incoming_flux = heat_flux_to_fluid_left_rad
    surface_temperature = interface_temperature_solid_left_rad
  []

  [solid_left]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = left_rad
    h = ${h_s}
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_left_rad
    surface_temperature = interface_temperature_fluid_left_rad
  []

  # Robin-Robin CHT at the right gap/solid interface.
  [fluid_right]
    type = LinearFVRobinCHTBC
    variable = T_fluid
    boundary = right_rad
    h = ${h_f}
    thermal_conductivity = ${k_fluid}
    incoming_flux = heat_flux_to_fluid_right_rad
    surface_temperature = interface_temperature_solid_right_rad
  []

  [solid_right]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = right_rad
    h = ${h_s}
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_right_rad
    surface_temperature = interface_temperature_fluid_right_rad
  []

  # Do not add LinearFVGrayLambertBC on left_rad/right_rad. The modified
  # CHTHandler removes q_rad from the flux transferred through the transparent
  # fluid and applies the corresponding surface loss/gain to the solids.
[]

[Postprocessors]
  # Interface temperatures
  [T_left_rad]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = TEMPERATURE
    boundary = left_rad
    execute_on = NONLINEAR
  []

  [T_right_rad]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = TEMPERATURE
    boundary = right_rad
    execute_on = NONLINEAR
  []

  # Radiative heat flux, positive from left to right
  [q_radiation]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = HEAT_FLUX_DENSITY
    boundary = left_rad
    execute_on = NONLINEAR
  []

  # Total heat flux entering through the hot external boundary
  [q_hot_boundary_outward]
    type = BoundaryLinearFVFluxIntegral
    boundary = left
    linearfvkernels = solid_conduction
    execute_on = NONLINEAR
  []

  [q_total]
    type = ParsedPostprocessor
    pp_names = 'q_hot_boundary_outward'
    expression = '-q_hot_boundary_outward'
    execute_on = NONLINEAR
  []

  # Conductive contribution through the middle region
  [q_conduction]
    type = ParsedPostprocessor
    pp_names = 'q_total q_radiation'
    expression = 'q_total-q_radiation'
    execute_on = NONLINEAR
  []

  # Analytical-solution errors
  [T_left_rel_error]
    type = ParsedPostprocessor
    pp_names = 'T_left_rad'
    expression = 'abs(T_left_rad-${T_left_ref})/${T_left_ref}'
    execute_on = NONLINEAR
  []

  [T_right_rel_error]
    type = ParsedPostprocessor
    pp_names = 'T_right_rad'
    expression = 'abs(T_right_rad-${T_right_ref})/${T_right_ref}'
    execute_on = NONLINEAR
  []

  [q_radiation_rel_error]
    type = ParsedPostprocessor
    pp_names = 'q_radiation'
    expression = 'abs(q_radiation-${q_radiation_ref})/${q_radiation_ref}'
    execute_on = NONLINEAR
  []

  [q_conduction_rel_error]
    type = ParsedPostprocessor
    pp_names = 'q_conduction'
    expression = 'abs(q_conduction-${q_conduction_ref})/${q_conduction_ref}'
    execute_on = NONLINEAR
  []

  [q_total_rel_error]
    type = ParsedPostprocessor
    pp_names = 'q_total'
    expression = 'abs(q_total-${q_total_ref})/${q_total_ref}'
    execute_on = NONLINEAR
  []
[]

[Executioner]
  type = SIMPLE
  num_iterations = 1000

  should_solve_momentum = false
  should_solve_pressure = false

  rhie_chow_user_object = rc
  momentum_systems = 'u_system v_system'
  pressure_system = p_system

  energy_system = energy_system
  solid_energy_system = solid_energy_system

  energy_l_abs_tol = 1e-14
  energy_l_tol = 1e-14
  energy_equation_relaxation = 0.99
  energy_field_relaxation = 0.99
  energy_absolute_tolerance = 1e-12

  solid_energy_l_abs_tol = 1e-14
  solid_energy_l_tol = 1e-14
  solid_energy_absolute_tolerance = 1e-12

  energy_petsc_options_iname = '-pc_type'
  energy_petsc_options_value = 'lu'
  solid_energy_petsc_options_iname = '-pc_type'
  solid_energy_petsc_options_value = 'lu'

  cht_interfaces = 'left_rad right_rad'
  cht_solid_flux_relaxation = '0.5 0.5'
  cht_fluid_flux_relaxation = '0.5 0.5'
  cht_solid_temperature_relaxation = '0.5 0.5'
  cht_fluid_temperature_relaxation = '0.5 0.5'
  cht_heat_flux_tolerance = 1e-10
  max_cht_fpi = 10

  # Parameter supplied by the radiation-aware CHTHandler.
  surface_radiation_object_name = gray_lambert

  print_fields = false
  continue_on_max_its = false
[]

[Outputs]
  execute_on = FINAL
  exodus = true
  csv = true
[]
