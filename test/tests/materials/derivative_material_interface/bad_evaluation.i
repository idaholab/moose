[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusion
    variable = u
    diffusivity = F
  [../]
[]

[Materials]
  [./time]
    type = GenericFunctionMaterial
    prop_names = 'time'
    prop_values = 't'
    outputs = all
  [../]

  [./F]
    type = DerivativeParsedMaterial
    property_name = F
    material_property_names = 'time'
    expression = 'if (time < 1.9, 1, log(-1))'
    disable_fpoptimizer = true
    enable_jit = false
    evalerror_behavior = nan
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
[]
