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
    type = MatDiffusion
    variable = dummy
    diffusivity = dummy_prop
  [../]
[]

[Materials]
  [./matprop]
    type = TemplateStateful
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
