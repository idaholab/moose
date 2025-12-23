# The test gurantees that the P1 radiation model in the Linear FV system matches the analytical solution
# for the incident radiation G under adiabatic and radiative interaction with the environment.
# This simulation is a 1D test with adiabatic Neumann BC on the left of the domain and a Marshak BC on
# the right of the domain, with varying wall temperatures.
# Assuming isothermal conditions along the volume, the analytical solution for this problem is:
# G(x) = G_bc * cosh(theta x) + 4*sigma*T^4, where:
# -theta = sqrt(absorption_coeff/diffusion_coef)
# G_bc = (4*sigma*(pow(wall_temperature,4)-pow(temperature_radiation,4))/(2*diffusion_coef*theta*sinh(theta)+cosh(theta)))

diffusion_coef = 1e-1
sigma_a = 1.0
temperature_radiation = 100.0
wall_temperature = 100.0
theta = ${fparse sqrt(sigma_a/diffusion_coef)}
eps_w = 1.0
G_bc = ${fparse 4*sigma*(pow(wall_temperature,4)-pow(temperature_radiation,4))/
(2*(2-eps_w)/eps_w*diffusion_coef*theta*sinh(theta)+cosh(theta))}
sigma = 5.670374419e-8

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 50
    xmin = 0
    xmax = 1
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
    initial_condition = 1
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
  [G_diffusion]
    type = LinearFVDiffusion
    variable = G
    diffusion_coeff = ${diffusion_coef}
  []
  [source_and_sink]
    type = LinearFVP1RadiationSourceSink
    variable = G
    temperature_radiation = ${temperature_radiation}
    absorption_coeff = ${sigma_a}
  []
[]

[LinearFVBCs]
  [right_bc]
    type = LinearFVP1RadiationMarshakBC
    boundary = 'right'
    variable = G
    temperature_radiation = ${wall_temperature}
    coeff_diffusion = ${diffusion_coef}
    boundary_emissivity = ${eps_w}
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
  [mean_value]
    type = ElementIntegralFunctorPostprocessor
    functor = G
  []
  [max_value]
    type = ElementExtremeFunctorValue
    functor = G
  []
  [mean_value_analytic]
    type = ElementIntegralFunctorPostprocessor
    functor = analytical_sol
  []
  [relative_difference]
    type = RelativeDifferencePostprocessor
    value1 = mean_value
    value2 = mean_value_analytic
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_abs_tol = 1e-12
[]

[Outputs]
  file_base = rad_isothermal_medium_1d_adiabatic
  csv = true
  execute_on = timestep_end
[]
