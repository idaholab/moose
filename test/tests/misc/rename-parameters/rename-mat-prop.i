[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    nx = 20
    dim = 1
  []
[]

[Variables]
  [u][]
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = 2
  []
  [rxn]
    type = RenamedMatReaction
    variable = u
    reaction_coefficient = 'rxn_coeff'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Materials]
  [diff]
    type = GenericConstantMaterial
    prop_names = 'rxn_coeff'
    prop_values = '-2'
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [avg_u]
    type = ElementAverageValue
    variable = u
  []
[]

[Outputs]
  csv = true
[]
