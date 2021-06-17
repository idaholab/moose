[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables/u]
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
  []
  [diff2]
    type = MatDiffusion
    variable = u
    diffusivity = 'andrew'
  []
[]

[Materials]
  [block]
    type = GenericConstantMaterial
    block = '0'
    prop_names = 'D andrew'
    prop_values = '1 1980'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [debug] # This is only a test, you should turn this on via [Debug] block
    type = MaterialPropertyDebugOutput
  []
[]
