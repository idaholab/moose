[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./dummy]
  [../]
[]

[Kernels]
  [./diff]
    type = ADMatDiffusion
    variable = dummy
    diffusivity = dummy_prop
  [../]
[]

[Materials]
  [./matprop]
    type = ADTemplateStateful
    property_name = dummy_prop
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
