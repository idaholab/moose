[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    block = 0
  []
[]

[AuxVariables]
  [u_residual]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
[]

[AuxKernels]
  [debug_aux]
    type = DebugResidualAux
    debug_variable = u
    variable = u_residual
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 10
  dt = 10

  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

