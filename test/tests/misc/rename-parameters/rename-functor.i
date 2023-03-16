[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Variables]
  [v]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [diff]
    type = RenamedCoeffFVDiffusion
    variable = v
    diffusion_coeff = diff
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 7
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 42
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'diff'
    prop_values = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [avg]
    type = ElementAverageValue
    variable = v
  []
[]
