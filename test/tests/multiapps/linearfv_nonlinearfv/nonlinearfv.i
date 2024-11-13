[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 6
  []
[]

[Variables]
  [v]
    type = MooseVariableFVReal
    initial_condition = 2.0
  []
[]

[AuxVariables]
  [diff_var]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
[]

[FVKernels]
  [diffusion]
    type = FVDiffusion
    variable = v
    coeff = diff_var
  []
  [source]
    type = FVBodyForce
    variable = v
    function = 3
  []
[]

[FVBCs]
  [dir]
    type = FVFunctorDirichletBC
    variable = v
    boundary = "left right"
    functor = 2
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
