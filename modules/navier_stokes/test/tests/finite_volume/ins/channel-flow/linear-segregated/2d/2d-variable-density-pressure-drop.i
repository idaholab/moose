# Verification case for the linear segregated 2D inlet/outlet solve with a prescribed
# density variation across the channel length.
#
# The intent is to exercise the same 2D SIMPLE + Rhie-Chow machinery used by the
# existing inlet/outlet channel cases, while keeping the exact solution simple enough
# to reason about on paper.
#
# This is deliberately not a fully closed compressible model. The density is imposed as
# a space-dependent functor, which makes this a clean verification problem for the
# variable-density momentum and pressure-correction pieces.
#
# Geometry and boundary-condition choice:
# - The mesh is 2D so the case still goes through the 2D linear-FV code path.
# - There is only one cell in the transverse direction.
# - The top and bottom boundaries are symmetry planes.
# Together those choices make the exact solution one-dimensional in x.
#
# Prescribed density:
#   rho(x) = rho_in + (rho_out - rho_in) * x / L
#
# With no body force and no viscous contribution (mu = 0), the steady 1D x-momentum
# equation reduces to
#   d/dx (rho * u^2 + p) = 0
# while continuity gives
#   rho * u = m = rho_in * u_in.
#
# Therefore
#   u(x) = m / rho(x)
#   p(x) = p_out + m^2 * (1 / rho_out - 1 / rho(x))
#
# For the values below:
#   rho_in = 1
#   rho_out = 0.5
#   u_in = 1
#   p_out = 0
#
# we get
#   m = 1
#   u_out = 2
#   Delta p = p_left - p_right = 1
#
# The postprocessors at the bottom compare the numerical pressure drop and outlet
# velocity against those exact values. The auxiliary fields also write rho(x), u_exact(x),
# and p_exact(x) to the output file so the solution can be inspected directly.

length = 10.0
height = 1.0
nx = 100

rho_in = 1.0
rho_out = 0.5
u_in = 1.0
p_out = 0.0

# Set to zero so the expected pressure change comes only from acceleration caused by
# the prescribed density variation. This keeps the reference solution simple.
mu = 0.0

# Upwind is used here for robustness because the case is effectively inviscid.
advected_interp_method = 'average'

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2

    # A single rectangular block is enough. The mesh is only one cell high so the
    # solution stays effectively one-dimensional while still using the 2D solver path.
    dx = '${length}'
    dy = '${height}'
    ix = '${nx}'
    iy = '1'
  []

  # Keep numbering stable if this file is later turned into a regression test.
  allow_renumbering = false
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure

    # This case is the reason for the file: use a spatially varying density functor
    # in the standard segregated Rhie-Chow mass-flux construction.
    rho = 'rho'

    p_diffusion_kernel = p_diffusion
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = u_system

    # Start from the inlet speed. The exact solution accelerates as rho decreases.
    initial_condition = ${u_in}
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0.0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system

    # The absolute pressure level is set by the outlet pressure BC below.
    initial_condition = ${p_out}
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    mu = 'mu'
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    mu = 'mu'
    u = vel_x
    v = vel_y
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
  []

  # Standard pressure-gradient terms for the momentum predictor.
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = 'x'
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
    momentum_component = 'y'
  []

  # Standard pressure equation used by SIMPLE.
  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []
[]

[LinearFVBCs]
  # Fix the inflow velocity. Because rho varies, the solver must accelerate the flow
  # downstream to maintain a constant mass flux.
  [inlet_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_x
    functor = ${u_in}
  []
  [inlet_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_y
    functor = 0.0
  []

  # Leave the outlet velocity free and fix the outlet pressure. That gives a clean
  # analytical reference for the pressure drop across the domain.
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'right'
    variable = vel_x
    use_two_term_expansion = true
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'right'
    variable = vel_y
    use_two_term_expansion = true
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right'
    variable = pressure
    functor = ${p_out}
  []

  # Symmetry on the top and bottom removes wall friction and keeps the exact solution
  # one-dimensional. This is what makes the pressure-drop formula above applicable.
  [symmetry_u]
    type = LinearFVVelocitySymmetryBC
    boundary = 'top bottom'
    variable = vel_x
    u = vel_x
    v = vel_y
    momentum_component = x
  []
  [symmetry_v]
    type = LinearFVVelocitySymmetryBC
    boundary = 'top bottom'
    variable = vel_y
    u = vel_x
    v = vel_y
    momentum_component = y
  []
  [symmetry_p]
    type = LinearFVPressureSymmetryBC
    boundary = 'top bottom'
    variable = pressure
    HbyA_flux = 'HbyA'
  []

  # The inlet pressure should not be prescribed. Let the momentum balance determine it,
  # then compare the resulting pressure drop against the exact value.
  [inlet_pressure_extrapolation]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'left'
    variable = pressure
    use_two_term_expansion = true
  []
[]

[FunctorMaterials]
  [rho]
    type = ParsedFunctorMaterial
    property_name = 'rho'

    # Linear density decrease from left to right.
    expression = '${rho_in} + (${rho_out} - ${rho_in}) * x / ${length}'
  []
  [mu]
    type = ParsedFunctorMaterial
    property_name = 'mu'

    # Keep viscosity in functor form so the file matches the standard momentum-kernel
    # interface. The value is zero for this inviscid verification setup.
    expression = '${mu}'
  []
  [u_exact]
    type = ParsedFunctorMaterial
    property_name = 'u_exact'
    functor_names = 'rho'

    # Exact velocity from constant mass flux rho*u = rho_in*u_in.
    expression = '${rho_in} * ${u_in} / rho'
  []
  [p_exact]
    type = ParsedFunctorMaterial
    property_name = 'p_exact'
    functor_names = 'rho'

    # Exact pressure from rho*u^2 + p = constant with the outlet pressure as reference.
    expression = '${p_out} + (${rho_in} * ${u_in}) * (${rho_in} * ${u_in}) * (1.0 / ${rho_out} - 1.0 / rho)'
  []
[]

[AuxVariables]
  # Write the imposed density and the exact reference fields to the output file so the
  # numerical solution can be compared visually in Exodus/CSV.
  [rho_aux]
    type = MooseLinearVariableFVReal
  []
  [u_exact_aux]
    type = MooseLinearVariableFVReal
  []
  [p_exact_aux]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [assign_rho_aux]
    type = FunctorAux
    variable = rho_aux
    functor = rho
    execute_on = 'initial timestep_end'
  []
  [assign_u_exact_aux]
    type = FunctorAux
    variable = u_exact_aux
    functor = u_exact
    execute_on = 'initial timestep_end'
  []
  [assign_p_exact_aux]
    type = FunctorAux
    variable = p_exact_aux
    functor = p_exact
    execute_on = 'initial timestep_end'
  []
[]

[Postprocessors]
  # Measure the actual inlet and outlet pressures from the solved field.
  [p_left]
    type = SideAverageValue
    variable = pressure
    boundary = 'left'
  []
  [p_right]
    type = SideAverageValue
    variable = pressure
    boundary = 'right'
  []
  [delta_p]
    type = ParsedPostprocessor
    expression = 'p_left - p_right'
    pp_names = 'p_left p_right'
  []

  # For the chosen parameters the exact pressure drop is 1.0, but keep the full formula
  # here so the case stays self-consistent if rho_in, rho_out, or u_in are changed later.
  [delta_p_expected]
    type = Receiver
    default = ${fparse (rho_in*u_in)*(rho_in*u_in)*(1/rho_out - 1/rho_in)}
  []
  [delta_p_error]
    type = ParsedPostprocessor
    expression = 'delta_p - delta_p_expected'
    pp_names = 'delta_p delta_p_expected'
  []
  [delta_p_error_percent]
    type = ParsedPostprocessor
    expression = '100 * delta_p_error / delta_p_expected'
    pp_names = 'delta_p_error delta_p_expected'
  []

  # The exact outlet speed follows directly from constant mass flux.
  [u_right]
    type = SideAverageValue
    variable = vel_x
    boundary = 'right'
  []
  [u_right_expected]
    type = Receiver
    default = ${fparse rho_in*u_in/rho_out}
  []
  [u_right_error]
    type = ParsedPostprocessor
    expression = 'u_right - u_right_expected'
    pp_names = 'u_right u_right_expected'
  []
[]

[VectorPostprocessors]
  # Sample the centerline so the solved and exact fields can be overlaid directly.
  [centerline_solution]
    type = LineValueSampler
    variable = 'rho_aux vel_x u_exact_aux pressure p_exact_aux'
    start_point = '0 0.5 0'
    end_point = '${length} 0.5 0'
    num_points = 401
    sort_by = x
    execute_on = timestep_end
  []
[]

[Executioner]
  type = SIMPLE

  # Conservative relaxations help on this effectively inviscid case.
  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  momentum_l_tol = 0
  pressure_l_tol = 0

  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'

  momentum_equation_relaxation = 0.4
  pressure_variable_relaxation = 0.1
  num_iterations = 1500

  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10

  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'

  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  csv = true
  execute_on = timestep_end
[]
