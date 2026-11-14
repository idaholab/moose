# Manufactured solution for the coupled volume-fraction, momentum and reduced-pressure solve
# with a tanh interface of fixed thickness eps. derive_tanh_interface_mms.py defines the exact
# fields and writes the forcing functions in tanh_interface_functions.i.
#
# The velocity is a pulsating vortex that enters and leaves through the top boundary with zero
# net flux and vanishes on the other three walls; the interface is a wavy layer whose height
# oscillates in time. The top boundary is a pressure outlet carrying the exact total pressure,
# whose inlet-outlet conditions see genuine inflow and outflow, the other boundaries are no-slip
# walls. The step scales with the mesh, dt = 0.5 / N, so a refinement in N refines space and
# time together.
#
# With BDF2 the combined refinement is second order in the velocity, the reduced pressure and
# the volume fraction, both with equal phase properties and at a density ratio of 1000 under
# gravity (see test.py). Refining dt alone on a fixed mesh is a different measurement: momentum
# and pressure stay second order, but alpha is first order, from the splitting of the
# semi-implicit MULES update between the implicit upwind solve and the explicit correction (see
# ConservativeDiffuseInterfaceVOFMULESCorrector). That term is of order dt h and so hides inside
# the second order of the combined refinement.

rho_l = 1000
rho_g = 1
mu_l = 1
mu_g = 0.01
g = 9.81
c_alpha = 1
eps = 0.05
U0 = 0.125
P0 = 100

N = 40
dt = ${fparse 0.5 / N}
end_time = 0.5

!include tanh_interface_functions.i

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${N}
    ny = ${N}
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system alpha_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [ConservativeDiffuseInterfaceFlowSegregated]
      [flow]
        velocity_variable = 'vel_x vel_y'
        pressure_variable = 'pressure'

        compressibility = 'incompressible'
        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'
        gravity = '0 -${g} 0'
        volume_fraction_functor = 'alpha'

        initial_velocity = 'exact_u exact_v 0'
        initial_pressure = 'exact_p_rgh'

        wall_boundaries = 'left right bottom'
        momentum_wall_types = 'noslip noslip noslip'

        outlet_boundaries = 'top'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = 'exact_p_total'

        orthogonality_correction = false
        momentum_two_term_bc_expansion = false
        pressure_two_term_bc_expansion = false
        momentum_advection_interpolation = 'average'
      []
    []
    [ConservativeDiffuseInterfaceVOFSegregated]
      [vof]
        coupled_flow_physics = 'flow'
        passive_scalar_names = 'alpha'
        initial_scalar_variables = 'exact_alpha'
        # The manufactured source is split as MULES expects: the explicit part here, the
        # implicit sink coefficient in the reaction kernel below, so the donor solve stays
        # bounded (see derive_tanh_interface_mms.py)
        passive_scalar_source = 'forcing_alpha_explicit'
        system_names = 'alpha_system'

        liquid_density_name = 'rho_l'
        gas_density_name = 'rho_g'
        liquid_dynamic_viscosity_name = 'mu_l'
        gas_dynamic_viscosity_name = 'mu_g'

        passive_scalar_advection_interpolation = 'upwind'
        compression_factor = '${c_alpha}'
        interface_normal_functor = 'flow_interface_unit_normal_face'

        n_alpha_corrections = 1
        n_limiter_iterations = 6
      []
    []
  []
[]

[LinearFVKernels]
  [forcing_alpha_implicit]
    type = LinearFVReaction
    variable = alpha
    coeff = forcing_alpha_implicit_coeff
  []
  [forcing_u]
    type = LinearFVSource
    variable = vel_x
    source_density = forcing_u
  []
  [forcing_v]
    type = LinearFVSource
    variable = vel_y
    source_density = forcing_v
  []
[]

[FunctorMaterials]
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'rho_l rho_g mu_l mu_g'
    prop_values = '${rho_l} ${rho_g} ${mu_l} ${mu_g}'
  []
[]

[Executioner]
  type = ReducedPressurePIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  [TimeIntegrator]
    type = BDF2
  []

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  active_scalar_systems = 'alpha_system'

  # With dt proportional to h the time-derivative diagonal grows like N while the pressure
  # coupling stiffens like N^2, so on fine meshes the outer loop approaches the steady SIMPLE
  # limit; 0.9/0.7 diverges at N = 160 and the usual SIMPLE values converge at every N
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  active_scalar_equation_relaxation = '1.0'

  num_iterations = 100
  num_piso_iterations = 0
  continue_on_max_its = true
  print_fields = false

  momentum_absolute_tolerance = 1e-8
  pressure_absolute_tolerance = 1e-8
  active_scalar_absolute_tolerance = '1e-8'

  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  active_scalar_l_abs_tol = 1e-12

  momentum_l_tol = 0
  pressure_l_tol = 0
  active_scalar_l_tol = 0

  momentum_petsc_options_iname = '-pc_type -sub_pc_type'
  momentum_petsc_options_value = 'bjacobi ilu'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  active_scalar_petsc_options_iname = '-pc_type -sub_pc_type'
  active_scalar_petsc_options_value = 'bjacobi ilu'

  # BDF2 reads the older solution, which a subcycle cannot supply
  volume_fraction_subcycles = 1
  volume_fraction_max_courant = 10

  dt = ${dt}
  end_time = ${end_time}
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = FINAL
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = vel_x
    exact = exact_u
    execute_on = FINAL
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = vel_y
    exact = exact_v
    execute_on = FINAL
  []
  [L2p]
    type = ElementL2FunctorError
    approximate = pressure
    exact = exact_p_rgh
    execute_on = FINAL
  []
  [L2alpha]
    type = ElementL2FunctorError
    approximate = alpha
    exact = exact_alpha
    execute_on = FINAL
  []
  [alpha_min]
    type = ElementExtremeValue
    variable = alpha
    value_type = min
    execute_on = FINAL
  []
  [alpha_max]
    type = ElementExtremeValue
    variable = alpha
    value_type = max
    execute_on = FINAL
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]
