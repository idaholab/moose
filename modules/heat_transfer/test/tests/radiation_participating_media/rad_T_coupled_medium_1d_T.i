# The test gurantees that the P1 radiation model in the Linear FV system using the Newton solver
# can be used with MultiApps to relax the solution update via relaxation to improve convergence
# of the coupled energy and radiative systems.
# This simulation is a 1D test with Dirichlet BCs on the left and right of the domain. Marshak BCs are
# applied at the boundaries for G.


k = 1.0
sigma_a = 0.33333
r_wall_temp = 400.0
l_wall_temp = 300.0

[Mesh]
  [salt_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 100
    xmin = -0.5
    xmax = 0.5
  []
[]

[Problem]
  linear_sys_names = 'energy_system'
  previous_nl_solution_required = true
[]

[Variables]
  [T]
    type = MooseLinearVariableFVReal
    solver_sys = 'energy_system'
    initial_condition = 350
  []
[]

[AuxVariables]
  [G]
    type = MooseLinearVariableFVReal
    initial_condition = 3400
  []
[]

[LinearFVKernels]
  [temp_conduction]
    type = LinearFVDiffusion
    diffusion_coeff = ${k}
    variable = T
  []
  [temp_radiation]
    type = LinearFVP1TemperatureSourceSink
    variable = T
    G = 'G'
    absorption_coeff = ${sigma_a}
  []
[]

[LinearFVBCs]
  [left_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'left'
    functor = ${l_wall_temp}
  []
  [right_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'right'
    functor = ${r_wall_temp}
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
  l_abs_tol = 1e-14
  l_tol = 1e-14
  nl_abs_tol = 1e-14
  fixed_point_min_its = 2
  fixed_point_max_its = 500
  relaxation_factor = 0.9
  transformed_variables = 'T'
  fixed_point_rel_tol = 1e-12
  fixed_point_abs_tol = 1e-12
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = timestep_end
[]

[MultiApps]
  [sub_app]
    type = FullSolveMultiApp
    input_files = 'rad_T_coupled_medium_1d_G.i'
    relaxation_factor = 0.9
  []
[]

[Transfers]
  [push]
    type = MultiAppCopyTransfer
    to_multi_app = sub_app
    source_variable = 'T'
    variable = 'T'
  []
  [pull]
    type = MultiAppCopyTransfer
    from_multi_app = sub_app
    source_variable = 'G'
    variable = 'G'
  []
[]
