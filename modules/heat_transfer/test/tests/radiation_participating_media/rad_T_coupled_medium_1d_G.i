# The test gurantees that the P1 radiation model in the Linear FV system using the simple
# multi-linear system solver can be used with MultiApps to relax the solution update via
# relaxation to improve convergence of the coupled energy and radiative systems.
# This simulation is a 1D test with Dirichlet BCs on the left and right of the domain. Marshak BCs are
# applied at the boundaries for G.

sigma_a = 0.33333
diffusion_coef = ${fparse 1/(3*sigma_a)}
b_eps = 1.0

r_wall_temp = 400.0
l_wall_temp = 300.0

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 100
    xmin = -0.5
    xmax = 0.5
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
    initial_condition = 3400
  []
[]

[AuxVariables]
  [T]
    type = MooseLinearVariableFVReal
    initial_condition = 350
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
  [right_bc_G]
    type = LinearFVP1RadiationMarshakBC
    boundary = 'right'
    variable = G
    temperature_radiation = ${r_wall_temp}
    coeff_diffusion = ${diffusion_coef}
    boundary_emissivity = ${b_eps}
  []
  [left_bc_G]
    type = LinearFVP1RadiationMarshakBC
    boundary = 'left'
    variable = G
    temperature_radiation = ${l_wall_temp}
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

[Executioner]
  type = Steady
  solve_type = 'Linear'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  l_abs_tol = 1e-16
  l_tol = 1e-16
  nl_abs_tol = 1e-16
  relaxation_factor = 0.9
[]
