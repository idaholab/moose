# The test gurantees that the P1 radiation model in the Linear FV system matches the analytical solution
# for the incident radiation G under adiabatic and radiative interaction with the environment.
# This simulation is a 1D test with adiabatic Neumann BC on the left of the domain and a Marshak BC on
# the right of the domain, with varying wall temperatures.
# Assuming isothermal conditions along the volume, the analytical solution for this problem is:
# G(x) = G_bc * cosh(theta x) + 4*sigma*T^4, where:
# -theta = sqrt(absorption_coeff/diffusion_coef)
# G_bc = (4*sigma*(pow(wall_temperature,4)-pow(temperature_radiation,4))/(2*diffusion_coef*theta*sinh(theta)+cosh(theta)))

#k = 1e-0
sigma_a = 1.0
diffusion_coef = ${fparse 1/(3*sigma_a)}

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
  linear_sys_names = 'radiation_system'
  previous_nl_solution_required = true
[]

[Variables]
  [G]
    type = MooseLinearVariableFVReal
    solver_sys = 'radiation_system'
    initial_condition = 850 #22.6815
  []
[]

[AuxVariables]
  [G_analytic]
    type = MooseLinearVariableFVReal
  []
  [T]
    type = MooseLinearVariableFVReal
    initial_condition = 350
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

  # [temp_conduction]
  #   type = LinearFVDiffusion
  #   diffusion_coeff = ${k}
  #   variable = T
  # []
  # [temp_radiation]
  #   type = LinearFVP1TemperatureSourceSink
  #   variable = T
  #   G = 'G'
  #   absorption_coeff = ${sigma_a}
  # []

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
    temperature_radiation = 'T'
    coeff_diffusion = ${diffusion_coef}
    boundary_emissivity = 1.0
  []
  [left_bc_G]
    type = LinearFVP1RadiationMarshakBC
    boundary = 'left'
    variable = G
    temperature_radiation = 'T'
    coeff_diffusion = ${diffusion_coef}
    boundary_emissivity = 1.0
  []
  # [right_bc_G]
  #   type = LinearFVAdvectionDiffusionFunctorDirichletBC
  #   variable = G
  #   boundary = 'right'
  #   functor = 22.6815
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
  relaxation_factor = 1
[]

# [Outputs]
#   #file_base = rad_isothermal_medium_1d_adiabatic
#   csv = true
#   exodus = true
#   execute_on = timestep_end
# []
