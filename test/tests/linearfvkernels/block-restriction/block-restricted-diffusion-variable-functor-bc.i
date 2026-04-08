[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 1
    dx = '0.5 0.5'
    ix = '5 5'
    subdomain_id = '1 2'
  []
  [create_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = cmg
    new_boundary = interface
    primary_block = 2
    paired_block = 1
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 0.0
    block = 2
  []
[]

[AuxVariables]
  [v]
    type = MooseLinearVariableFVReal
    initial_condition = 1.0
    block = 1
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = 1
    use_nonorthogonal_correction = false
    block = 2
  []
[]

[LinearFVBCs]
  [interface]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = interface
    functor = v
  []
  [right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = right
    functor = 0
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[VectorPostprocessors]
  [u_sample]
    type = LineValueSampler
    variable = u
    start_point = '0.55 0 0'
    end_point = '0.95 0 0'
    num_points = 5
    sort_by = x
  []
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
