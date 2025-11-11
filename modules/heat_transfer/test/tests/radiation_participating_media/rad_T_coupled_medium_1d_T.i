# The test gurantees that the P1 radiation model in the Linear FV system matches the analytical solution
# for the incident radiation G under adiabatic and radiative interaction with the environment.
# This simulation is a 1D test with adiabatic Neumann BC on the left of the domain and a Marshak BC on
# the right of the domain, with varying wall temperatures.
# Assuming isothermal conditions along the volume, the analytical solution for this problem is:
# G(x) = G_bc * cosh(theta x) + 4*sigma*T^4, where:
# -theta = sqrt(absorption_coeff/diffusion_coef)
# G_bc = (4*sigma*(pow(wall_temperature,4)-pow(temperature_radiation,4))/(2*diffusion_coef*theta*sinh(theta)+cosh(theta)))

k = 1.0
diffusion_coef = 1e-3
sigma_a = 1.0
temperature_radiation = 100.0
wall_temperature = 100.0
theta = ${fparse sqrt(sigma_a/diffusion_coef)}
G_bc = ${fparse 4*sigma*(pow(wall_temperature,4)-pow(temperature_radiation,4))/(2*diffusion_coef*theta*sinh(theta)+cosh(theta))}
sigma = 5.670374419e-8


[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 50
    xmin = 0
    xmax = 1
  []
  # [block_1]
  #   type = GeneratedMeshGenerator
  #   dim = 1
  #   xmin = 0
  #   xmax = 0.5
  #   nx = 50
  #   bias_x = ${fparse 1/0.95}
  # []
  # [block_2]
  #   type = GeneratedMeshGenerator
  #   dim = 1
  #   xmin = 0.5
  #   xmax = 1.0
  #   nx = 50
  #   bias_x = 0.95
  # []
  # [smg]
  #   type = StitchedMeshGenerator
  #   inputs = 'block_1 block_2'
  #   clear_stitched_boundary_ids = true
  #   stitch_boundaries_pairs = 'right left'
  #   merge_boundaries_with_same_name = true
  # []
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
  [G_analytic]
    type = MooseLinearVariableFVReal
  []
  [G]
    type = MooseLinearVariableFVReal
    initial_condition = 850 #22.6815
  []
[]

[AuxKernels]
  [populate_analytical]
    type = FunctorAux
    functor = analytical_sol
    variable = G_analytic
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

  # [G_diffusion]
  #   type = LinearFVDiffusion
  #   variable = G
  #   diffusion_coeff = ${diffusion_coef}
  # []
  # [source_and_sink]
  #   type = LinearFVP1RadiationSourceSink
  #   variable = G
  #   temperature_radiation = 'T'
  #   absorption_coeff = ${sigma_a}
  # []
[]

[LinearFVBCs]
  # [right_bc_G]
  #   type = LinearFVP1RadiationMarshakBC
  #   boundary = 'right'
  #   variable = G
  #   temperature_radiation = 'T'
  #   coeff_diffusion = ${diffusion_coef}
  #   boundary_emissivity = 1.0
  # []
  # [right_bc_G]
  #   type = LinearFVAdvectionDiffusionFunctorDirichletBC
  #   variable = G
  #   boundary = 'right'
  #   functor = 22.6815
  # []
  # [left_bc_T_marshak]
  #   type = LinearFVP1TemperatureMarshakBC
  #   variable = T
  #   boundary = 'left'
  #   temperature_radiation = 300.
  #   G = 'G'
  #   coeff_diffusion = ${k}
  #   boundary_emissivity = 1.0
  # []
  # [right_bc_T_marshak]
  #   type = LinearFVP1TemperatureMarshakBC
  #   variable = T
  #   boundary = 'right'
  #   temperature_radiation = 400.
  #   G = 'G'
  #   coeff_diffusion = ${k}
  #   boundary_emissivity = 1.0
  # []
  [left_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'left'
    functor = 300.
  []
  [right_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'right'
    functor = 400.
  []
[]

[Functions]
  [analytical_sol]
    type = ParsedFunction
    symbol_names = 'a'
    symbol_values = '${fparse sqrt(sigma_a / diffusion_coef)}'
    expression = '${G_bc} * cosh(${theta}*x) + 4* ${sigma} * ${temperature_radiation}^4'
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
#   [fp_conv]
#     type = IterationCountConvergence
#     max_iterations = 100
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
  l_abs_tol = 1e-14
  l_tol = 1e-14
  nl_abs_tol = 1e-14
  fixed_point_min_its = 2
  fixed_point_max_its = 200
  relaxation_factor = 1.
  fixed_point_rel_tol = 1e-14
  fixed_point_abs_tol = 1e-14
  # multiapp_fixed_point_convergence = fp_conv
  # multi_system_fixed_point=true
  # multi_system_fixed_point_convergence=linear
[]

[Outputs]
  #file_base = rad_isothermal_medium_1d_adiabatic
  csv = true
  exodus = true
  execute_on = timestep_end
[]

[MultiApps]
  [sub_app]
    type = FullSolveMultiApp
    input_files = 'rad_T_coupled_medium_1d_G.i'
    #execute_on = timestep_end
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
