# The test gurantees that the P1 radiation model in the Linear FV system matches the analytical solution
# for the incident radiation G under adiabatic and radiative interaction with the environment.
# This simulation is a 1D test with adiabatic Neumann BC on the left of the domain and a Marshak BC on
# the right of the domain, with varying wall temperatures.
# Assuming isothermal conditions along the volume, the analytical solution for this problem is:
# G(x) = G_bc * cosh(theta x) + 4*sigma*T^4, where:
# -theta = sqrt(absorption_coeff/diffusion_coef)
# G_bc = (4*sigma*(pow(wall_temperature,4)-pow(temperature_radiation,4))/(2*diffusion_coef*theta*sinh(theta)+cosh(theta)))

sigma_a = 1.0
diffusion_coef = ${fparse 1/(3*sigma_a)}
b_eps = 1.0
T_w = 100.



[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 50
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
[]

[Problem]
  linear_sys_names = 'radiation_system'
  previous_nl_solution_required = true
[]

[Variables]
  [G]
    type = MooseLinearVariableFVReal
    solver_sys = 'radiation_system'
    initial_condition = 1 #22.6815
  []
[]

[AuxVariables]
  [T]
    type = MooseLinearVariableFVReal
    initial_condition = 1
  []
[]

[LinearFVKernels]
  [G_diffusion]
    type = LinearFVDiffusion
    variable = G
    diffusion_coeff = ${diffusion_coef}
  []
  [source_and_sink]
    type = LinearFVP1RadiationSourceSink
    variable = G
    temperature_radiation = 'T'
    absorption_coeff = ${sigma_a}
  []
[]

[LinearFVBCs]
  [otherwalls_bc_G]
    type = LinearFVP1RadiationMarshakBC
    boundary = 'right left top'
    variable = G
    temperature_radiation = ${fparse 0.5*T_w}
    coeff_diffusion = ${diffusion_coef}
    boundary_emissivity = ${b_eps}
  []
  [bottom_bc_G]
    type = LinearFVP1RadiationMarshakBC
    boundary = 'bottom'
    variable = G
    temperature_radiation =  ${fparse T_w}
    coeff_diffusion = ${diffusion_coef}
    boundary_emissivity = ${b_eps}
  []
[]

[Postprocessors]
  [mean_value_G]
    type = ElementIntegralFunctorPostprocessor
    functor = G
  []
  [max_value_G]
    type = ElementExtremeFunctorValue
    functor = G
  []
  [mean_value_T]
    type = ElementIntegralFunctorPostprocessor
    functor = T
  []
  [max_value_T]
    type = ElementExtremeFunctorValue
    functor = T
  []
[]

# [Convergence]
#   [linear]
#     type = IterationCountConvergence
#     max_iterations = 2000
#     converge_at_max_iterations = true
#   []
# []

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  # petsc_options_iname = '-pc_type -pc_factor_shift_type'
  # petsc_options_value = 'lu NONZERO'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  l_abs_tol = 1e-16
  l_tol = 1e-16
  nl_abs_tol = 1e-16
  relaxation_factor = 0.8
  transformed_variables = 'G'
[]

# [Outputs]
#   #file_base = rad_isothermal_medium_1d_adiabatic
#   csv = true
#   exodus = true
#   execute_on = timestep_end
# []
