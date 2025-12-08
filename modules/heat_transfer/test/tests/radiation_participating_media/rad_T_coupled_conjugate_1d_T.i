# The test gurantees that the P1 radiation model in the Linear FV system matches the analytical solution
# for the incident radiation G under adiabatic and radiative interaction with the environment.
# This simulation is a 1D test with adiabatic Neumann BC on the left of the domain and a Marshak BC on
# the right of the domain, with varying wall temperatures.
# Assuming isothermal conditions along the volume, the analytical solution for this problem is:
# G(x) = G_bc * cosh(theta x) + 4*sigma*T^4, where:
# -theta = sqrt(absorption_coeff/diffusion_coef)
# G_bc = (4*sigma*(pow(wall_temperature,4)-pow(temperature_radiation,4))/(2*diffusion_coef*theta*sinh(theta)+cosh(theta)))

Tw_left = 1.0
Tw_right = 0.0
sigma = 5.670374419e-8

##### tau = 0.1 ; 1.0;  10.0 ######
sigma_a = 1.0 # = tau because L=1

##### N = 0.01 ; 0.1 ; 1.0 ; inf #####
N = 0.1 # Stark number N = k sigma_a / (4*sigma*pow(T_w,3))
k_salt = ${fparse N*4*sigma*pow(Tw_left,3)/sigma_a}
k_solid = ${fparse 4.0*k_salt}

#b_eps = 1.0

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
  linear_sys_names = 'energy_system'
  previous_nl_solution_required = true
[]

[Variables]
  [T]
    type = MooseLinearVariableFVReal
    solver_sys = 'energy_system'
    initial_condition = ${fparse Tw_left/2}
  []
[]

[AuxVariables]
  [G]
    type = MooseLinearVariableFVReal
    initial_condition = ${fparse 4*sigma*pow(Tw_left/2,4)} #22.6815
    block = salt
  []
[]

[AuxKernels]
  # [populate_analytical]
  #   type = FunctorAux
  #   functor = analytical_sol
  #   variable = G_analytic
  # []
[]

[LinearFVKernels]

  [temp_conduction]
    type = LinearFVDiffusion
    diffusion_coeff = k_total #${k_salt}
    variable = T
  []
  [temp_radiation]
    type = LinearFVP1TemperatureSourceSink
    variable = T
    G = 'G'
    absorption_coeff = ${sigma_a}
    block = salt
  []

  [source]
    type = LinearFVSource
    variable = T
    source_density = 'rad_source'
    block = solid
  []

  # [temp_conduction_sol]
  #   type = LinearFVDiffusion
  #   diffusion_coeff = k_total #${k_solid}
  #   variable = T
  #   block = solid
  # []
[]

[FunctorMaterials]
  [k_total]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = k_total
    subdomain_to_prop_value = 'salt ${k_salt}
                               solid ${k_solid}'
  []
[]

[LinearFVBCs]
  [left_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'left'
    functor = ${Tw_left}
  []
  [right_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'right'
    functor = ${Tw_right}
  []
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
#   [fp_conv]
#     type = IterationCountConvergence
#     max_iterations = 100
#     converge_at_max_iterations = true
#   []
# []

[FunctorMaterials]
  [solid_source]
    type = ParsedFunctorMaterial
    property_name = 'rad_source'
    expression = 'if(x < 1.01 , 0. * 0.000000024/0.0125, 0.0)'
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
  fixed_point_max_its = 400
  relaxation_factor = 0.9
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
    input_files = 'rad_T_coupled_conjugate_1d_G.i'
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
