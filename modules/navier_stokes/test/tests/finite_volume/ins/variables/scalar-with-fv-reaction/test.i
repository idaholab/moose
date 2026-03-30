[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
  []
[]

[Variables]
  [u]
    type = INSFVScalarFieldVariable
  []
[]

[FVKernels]
  [time]
    type = FVDiffusion
    coeff = 1
    variable = u
  []
  [rxn]
    type = FVReaction
    variable = u
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    boundary = 'left'
    value = 1
    variable = u
  []
  [right]
    type = FVDirichletBC
    boundary = 'right'
    value = 0
    variable = u
  []
[]

[Executioner]
  type = Steady
  solve_type = Newton
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Postprocessors]
  [avg]
    type = ElementIntegralFunctorPostprocessor
    functor = u
  []
[]

[Outputs]
  csv = true
[]
