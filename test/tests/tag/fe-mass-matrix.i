[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [mass]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [mass]
    type = MassMatrix
    variable = u
    matrix_tags = 'mass'
  []
[]

[AuxKernels]
  [TagMatrixAux1]
    type = TagMatrixAux
    variable = mass
    v = u
    matrix_tag = mass
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Problem]
  type = FEProblem
  extra_tag_matrices = 'mass'
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
