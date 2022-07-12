[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmax = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = MatDiffusion
    variable = u
    diffusivity = D
  []
  [absorption]
    type = MaterialReaction
    variable = u
    coefficient = sig
  []
  [source]
    type = BodyForce
    variable = u
    value = 1.0
  []
[]

[Materials]
  [diffusivity]
    type = GenericConstantMaterial
    prop_names = D
    prop_values = 2.0
  []
  [xs]
    type = GenericConstantMaterial
    prop_names = sig
    prop_values = 2.0
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
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[Postprocessors]
  [avg]
    type = AverageNodalVariableValue
    variable = u
  []
  [max]
    type = NodalExtremeValue
    variable = u
    value_type = max
  []
[]
