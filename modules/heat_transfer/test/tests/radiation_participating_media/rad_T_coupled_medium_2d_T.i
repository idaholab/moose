# The test gurantees that the P1 radiation model in the Linear FV system matches the analytical solution
# for the incident radiation G under adiabatic and radiative interaction with the environment.
# This simulation is a 1D test with adiabatic Neumann BC on the left of the domain and a Marshak BC on
# the right of the domain, with varying wall temperatures.
# Assuming isothermal conditions along the volume, the analytical solution for this problem is:
# G(x) = G_bc * cosh(theta x) + 4*sigma*T^4, where:
# -theta = sqrt(absorption_coeff/diffusion_coef)
# G_bc = (4*sigma*(pow(wall_temperature,4)-pow(temperature_radiation,4))/(2*diffusion_coef*theta*sinh(theta)+cosh(theta)))

#k = 0.0000002268148
b_eps = 1.0
sigma_a = 1.0
# diffusion_coef = ${fparse 1/(3*sigma_a)}
sigma = 5.670374419e-8

N = 0.01 # Stark number N = k sigma_a / (4*sigma*pow(T_w,3))
T_w = 100.
k = ${fparse N*4*sigma*pow(T_w,3)/sigma_a}

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
  linear_sys_names = 'energy_system'
  previous_nl_solution_required = true
[]

[Variables]
  [T]
    type = MooseLinearVariableFVReal
    solver_sys = 'energy_system'
    initial_condition = 1
  []
[]

[AuxVariables]
  [G]
    type = MooseLinearVariableFVReal
    initial_condition = 1 #22.6815
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
  [other_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'right left top'
    functor = ${fparse 0.5*T_w}
  []
  [bottom_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'bottom'
    functor = ${T_w}
  []
  # [left_bc_T]
  #   type = LinearFVP1TemperatureMarshakBC
  #   variable = T
  #   temperature_radiation = ${fparse 0.5*T_w}
  #   boundary = 'right left top'
  #   G = 'G'
  #   coeff_diffusion = ${k}
  #   boundary_emissivity = ${b_eps}
  # []
  # [right_bc_T]
  #   type = LinearFVP1TemperatureMarshakBC
  #   variable = T
  #   temperature_radiation = ${T_w}
  #   boundary = 'bottom'
  #   G = 'G'
  #   coeff_diffusion = ${k}
  #   boundary_emissivity = ${b_eps}
  # []
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

[FunctorMaterials]
  [parsed_source_right]
    type = ParsedFunctorMaterial
    expression = 'if(x>0.99, 0*${b_eps}/(2*(2-${b_eps})) *(4*5.67e-8*pow(400,4)-G),
                  if(x<0.01, 0*${b_eps}/(2*(2-${b_eps})) *(4*5.67e-8*pow(300,4)-G), 0.0 ))'
    property_name = 'emmision_source'
    functor_names = 'G'
  []
[]

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
  fixed_point_max_its = 500
  relaxation_factor = 0.8
  transformed_variables = 'T'
  fixed_point_rel_tol = 1e-12
  fixed_point_abs_tol = 1e-12
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
    input_files = 'rad_T_coupled_medium_2d_G.i'
    #execute_on = timestep_end
    relaxation_factor = 0.8
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
