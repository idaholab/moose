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
    type = RenamedPostprocessorDiffusion
    variable = u
    diffusion_postprocessor = 'parsed'
  []
  [rxn]
    type = Reaction
    rate = 2
    variable = u
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

[Executioner]
  type = Steady
[]

[Postprocessors]
  [parsed]
    type = ParsedPostprocessor
    pp_names = ''
    function = '2'
    execute_on = 'initial'
  []
  [avg_u]
    type = ElementAverageValue
    variable = u
  []
[]

[Outputs]
  [out]
    type = CSV
    hide = 'parsed'
  []
[]
