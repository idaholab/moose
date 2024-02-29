[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 1
    ix = '4 4'
    dx = '1 1'
    subdomain_id = '0 1'
  []
  [boundary]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    new_boundary = right_middle
    primary_block = 0
    paired_block = 1
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
  error_on_jacobian_nonzero_reallocation = true
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    sovler_sys = 'u_sys'
    initial_condition = 1.0
    block = 0
  []
[]

[LinearFVKernels]
  [advection]
    type = LinearFVAdvectionKernel
    variable = u
    velocity = "0.5 0 0"
    advected_interp_method = average
  []
  [diffusion]
    type = LinearFVDiffusionKernel
    variable = u
    diffusion_coeff = 3.0
  []
  [reaction]
    type = LinearFVReactionKernel
    variable = u
    coeff = 5.0
  []
  [source]
    type = LinearFVSourceKernel
    variable = u
    source_density = 1.5
  []
[]

[LinearFVBCs]
  [left_dir]
    type = LinearFVFunctorDirichletBC
    variable = u
    boundary = "left"
    functor = 1.0
  []
  [right_dir]
    type = LinearFVFunctorDirichletBC
    variable = u
    boundary = "right_middle"
    functor = 2.0
  []
[]

[Executioner]
  type = LinearPicardSteady
  linear_systems_to_solve = u_sys
  print_operators_and_vectors = true
  petsc_options_iname = "-pc_type -pc_hypre_type"
  petsc_options_value = "hypre boomeramg"
[]

[Outputs]
  exodus = true
  # [pgraph]
  #   type = PerfGraphOutput
  #   execute_on = 'final'  # Default is "final"
  #   level = 5                     # Default is 1
  #   heaviest_branch = false        # Default is false
  #   heaviest_sections = 10         # Default is 0
  # []
[]
