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
    type = MooseVariableFVReal
  []
[]

[AuxVariables]
  [mass]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
  [mass]
    type = FVMassMatrix
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

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = FVDirichletBC
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
