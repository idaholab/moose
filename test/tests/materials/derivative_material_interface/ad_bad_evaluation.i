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
    type = ADMatDiffusion
    variable = u
    diffusivity = F
  [../]
[]

[Materials]
  [./time_no_ad]
    type = GenericFunctionMaterial
    prop_names = 'time_no_ad'
    prop_values = 't'
    outputs = all
  [../]
  [./time]
    type = MaterialADConverter
    reg_props_in = time_no_ad
    ad_props_out = time
  [../]

  [./F]
    type = ADDerivativeParsedMaterial
    property_name = F
    material_property_names = 'time'
    expression = 'if (time < 1.9, 1, log(-1))'
    disable_fpoptimizer = true
    evalerror_behavior = nan
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
[]
