[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [v]
  []
[]

[AuxVariables]
  [u]
  []
[]

[Kernels]
  [diff_v]
    type = Diffusion
    variable = v
  []
  [force_v]
    type = CoupledForce
    variable = v
    v = u
  []
[]

[BCs]
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 1
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = FlexibleExecutioner
  solve_object_ordering = solve
  [solve]
    type = FEProblemSolve
    solve_type = PJFNK
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
    nl_abs_tol = 1e-10
  []
[]

[Outputs]
  exodus = true
[]

