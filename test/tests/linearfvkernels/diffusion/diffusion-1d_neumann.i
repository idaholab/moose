[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 1.0
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = 1
  []
[]

[LinearFVBCs]
  [dir]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left"
    functor = 300
  []
  [neu]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    variable = u
    boundary = "right"
    functor = -10
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_abs_tol = 1e-10
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
