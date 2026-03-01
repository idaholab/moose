# The test gurantees that the P1 participating media radiation model in the Linear FV
# system can couple with the energy equation of a semitransparent fluid with fixed
# temperatures at the left and right walls.
# The Newton solver approach is used. Due to the strong coupling between T and G
# fixed point iterations need to be set.

k = 1e-2
sigma_a = 1.
G_diffusion_coef = ${fparse 1/(3*sigma_a)}
sigma = 5.670374419e-8

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
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
    initial_condition = 100
  []
  [G]
    type = MooseLinearVariableFVReal
    solver_sys = 'radiation_system'
    initial_condition = ${fparse sigma*100^4}
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
    diffusion_coeff = ${G_diffusion_coef}
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
    coeff_diffusion = ${G_diffusion_coef}
    boundary_emissivity = 1.0
  []
  [right_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'right'
    functor = 100.
  []
  [left_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'left'
    functor = 50.
  []
  [left_bc_G]
    type = LinearFVP1RadiationMarshakBC
    boundary = 'left'
    variable = G
    temperature_radiation = 'T'
    coeff_diffusion = ${G_diffusion_coef}
    boundary_emissivity = 1.0
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
    max_iterations = 70
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'Linear' #'NEWTON'
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
