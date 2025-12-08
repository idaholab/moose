# The test gurantees that the P1 radiation model in the Linear FV system matches the analytical solution
# for the incident radiation G under adiabatic and radiative interaction with the environment.
# This simulation is a 1D test with adiabatic Neumann BC on the left of the domain and a Marshak BC on
# the right of the domain, with varying wall temperatures.
# Assuming isothermal conditions along the volume, the analytical solution for this problem is:
# G(x) = G_bc * cosh(theta x) + 4*sigma*T^4, where:
# -theta = sqrt(absorption_coeff/diffusion_coef)
# G_bc = (4*sigma*(pow(wall_temperature,4)-pow(temperature_radiation,4))/(2*diffusion_coef*theta*sinh(theta)+cosh(theta)))

k = 5e-3
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
    nx = 
    xmin = 0
    xmax = 1
  []
[]

[Problem]
  linear_sys_names = 'energy_system radiation_system'
  previous_nl_solution_required = true
[]

[Variables]
  [T]
    type = MooseLinearVariableFVReal
    solver_sys = 'energy_system'
    initial_condition = 20
  []
  [G]
    type = MooseLinearVariableFVReal
    solver_sys = 'radiation_system'
    initial_condition = 10 #22.6815
  []
[]

[AuxVariables]
  [G_analytic]
    type = MooseLinearVariableFVReal
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
  # [right_bc_G]
  #   type = LinearFVAdvectionDiffusionFunctorDirichletBC
  #   variable = G
  #   boundary = 'right'
  #   functor = 22.6815
  # []
  [right_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'right'
    functor = 100.
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

[Convergence]
  [linear]
    type = IterationCountConvergence
    max_iterations = 1200
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  # petsc_options_iname = '-energy_system_pc_type -energy_system_pc_factor_shift_type -energy_system_pc_type -energy_system_pc_factor_shift_type'
  # petsc_options_value = 'lu NONZERO'
  petsc_options_iname = '-energy_system_pc_type -energy_system_pc_factor_shift_type -radiation_system_pc_type -radiation_system_pc_factor_shift_type'
  petsc_options_value = 'hypre boomeramg hypre boomeramg'
  l_abs_tol = 1e-14
  l_tol = 1e-14
  nl_abs_tol = 1e-14
  multi_system_fixed_point=true
  multi_system_fixed_point_convergence=linear
[]

[Outputs]
  #file_base = rad_isothermal_medium_1d_adiabatic
  csv = true
  exodus = true
  execute_on = timestep_end
[]
