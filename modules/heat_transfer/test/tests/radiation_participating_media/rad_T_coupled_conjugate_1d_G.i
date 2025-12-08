# The test gurantees that the P1 radiation model in the Linear FV system matches the analytical solution
# for the incident radiation G under adiabatic and radiative interaction with the environment.
# This simulation is a 1D test with adiabatic Neumann BC on the left of the domain and a Marshak BC on
# the right of the domain, with varying wall temperatures.
# Assuming isothermal conditions along the volume, the analytical solution for this problem is:
# G(x) = G_bc * cosh(theta x) + 4*sigma*T^4, where:
# -theta = sqrt(absorption_coeff/diffusion_coef)
# G_bc = (4*sigma*(pow(wall_temperature,4)-pow(temperature_radiation,4))/(2*diffusion_coef*theta*sinh(theta)+cosh(theta)))

sigma = 5.670374419e-8

##### tau = 0.1 ; 1.0;  10.0 ######
sigma_a = 1.0 # = tau because L=1
diffusion_coef = ${fparse 1/(3*sigma_a)}
b_eps = 1.0
Tw_left = 1.0
#Tw_right = 0.0

[Mesh]
  [salt_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 80
    xmin = 0
    xmax = 1
    subdomain_ids = 0
  []
  [solid_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 80
    xmin = 1
    xmax = 2
    subdomain_ids = 1
  []
  [give_name_solid]
    type = RenameBlockGenerator
    input = solid_mesh
    old_block = 1
    new_block = 'solid'
  []
  [give_name_salt]
    type = RenameBlockGenerator
    input = salt_mesh
    old_block = 0
    new_block = 'salt'
  []
  [stitch]
    type = StitchMeshGenerator
    inputs = 'give_name_salt give_name_solid'
    stitch_boundaries_pairs = 'right left'
  []
  [interface]
    input = stitch
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 'solid'
    paired_block = 'salt'
    new_boundary = interface
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
    initial_condition = ${fparse 4*sigma*pow(Tw_left/2,4)}
    block = salt
  []
[]

[AuxVariables]
  [T]
    type = MooseLinearVariableFVReal
    initial_condition = ${fparse Tw_left*0.5}
  []
[]

[AuxKernels]
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
    block = salt
  []
  [source_and_sink]
    type = LinearFVP1RadiationSourceSink
    variable = G
    temperature_radiation = 'T'
    absorption_coeff = ${sigma_a}
    block = salt
  []
[]

[LinearFVBCs]
  [right_bc_G]
    type = LinearFVP1RadiationMarshakBC
    boundary = 'interface'
    variable = G
    temperature_radiation = 'T'
    coeff_diffusion = ${diffusion_coef}
    boundary_emissivity = ${b_eps}
  []
  [left_bc_G]
    type = LinearFVP1RadiationMarshakBC
    boundary = 'left'
    variable = G
    temperature_radiation = ${Tw_left}
    coeff_diffusion = ${diffusion_coef}
    boundary_emissivity = ${b_eps}
  []

  [left_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'left'
    functor = ${Tw_left}
  []
  # [right_bc_T]
  #   type = LinearFVAdvectionDiffusionFunctorDirichletBC
  #   variable = T
  #   boundary = 'right'
  #   functor = 400.
  # []
[]

[Postprocessors]
  # [mean_value_G]
  #   type = ElementIntegralFunctorPostprocessor
  #   functor = G
  # []
  # [max_value_G]
  #   type = ElementExtremeFunctorValue
  #   functor = G
  # []
  # [mean_value_T]
  #   type = ElementIntegralFunctorPostprocessor
  #   functor = T
  # []
  # [max_value_T]
  #   type = ElementExtremeFunctorValue
  #   functor = T
  # []
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
