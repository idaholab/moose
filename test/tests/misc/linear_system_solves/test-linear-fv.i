[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
[]

[Problem]
  linear_sys_names = 'u_sys'
  error_on_jacobian_nonzero_reallocation = true
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    linear_sys = 'u_sys'
    initial_condition = 1.0
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
    boundary = "right"
    functor = 2.0
  []
[]

[Executioner]
  type = LinearPicardSteady
  linear_sys_to_solve = u_sys
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
