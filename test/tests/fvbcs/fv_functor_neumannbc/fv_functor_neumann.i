[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 2
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    initial_condition = 0.5
  []
[]

[FVKernels]
  [diff_left]
    type = FVDiffusion
    variable = u
    coeff = 4
  []
[]

[AuxVariables]
  [qdot]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [set_qdot]
    type = FunctionIC
    variable = qdot
    function = 'y'
  []
[]

[FVBCs]
  [left]
    type = FVFunctorNeumannBC
    variable = u
    functor = qdot
    boundary = left
  []

  [right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
