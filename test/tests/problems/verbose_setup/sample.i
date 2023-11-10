[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [u]
    initial_condition = 3
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxVariables]
  [c]
  []
[]

[AuxKernels]
  [copy]
    type = ProjectionAux
    v = u
    variable = c
  []
[]

[Materials]
  [unused]
    type = GenericConstantMaterial
    prop_names = 'f1'
    prop_values = '2'
  []
[]

[Functions]
  [f]
    type = ConstantFunction
    value = 1
  []
[]

[Problem]
  type = FEProblem
  solve = false
  verbose_setup = true
[]

[Executioner]
  type = Steady
[]
