[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
  uniform_refine = 2
[]

[Variables]
  [./u1]
  [../]
  [./u2]
  [../]
  [./u3]
  [../]
[]

[Kernels]
  [./diff1]
    type = Diffusion
    variable = u1
  [../]
  [./diff2]
    type = Diffusion
    variable = u2
  [../]
  [./diff3]
    type = Diffusion
    variable = u3
  [../]

  [./dt1]
    type = TimeDerivative
    variable = u1
  [../]
  [./dt2]
    type = TimeDerivative
    variable = u2
  [../]
  [./dt3]
    type = TimeDerivative
    variable = u3
  [../]
[]

[DiracKernels]
  [./material_source1]
    type = MaterialMultiPointSource
    variable = u1
    points = '0.2 0.3 0.0
              0.7 0.5 0.0'
  [../]
  [./material_source2]
    type = MaterialMultiPointSource
    variable = u2
    points = '0.2 0.3 0.0
              0.2 0.3 0.0'
  [../]
  [./material_source3]
    type = MaterialMultiPointSource
    variable = u3
    drop_duplicate_points = false
    points = '0.2 0.3 0.0
              0.2 0.3 0.0'
  [../]
[]

[Postprocessors]
  [./u1]
    type = ElementIntegralVariablePostprocessor
    variable = u1
  [../]
  [./u2]
    type = ElementIntegralVariablePostprocessor
    variable = u2
  [../]
  [./u3]
    type = ElementIntegralVariablePostprocessor
    variable = u3
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    prop_names = matp
    prop_values = 1.0
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 1
[]

[Outputs]
  csv = true
  print_linear_residuals = false
[]
