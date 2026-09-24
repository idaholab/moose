[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  xmin = 0
  xmax = 30
  ymin = 0
  ymax = 30
[]

[Variables]
  [./c]
  [../]
  [./w]
  [../]
  [./eta1]
  [../]
  [./eta2]
  [../]
  [./eta3]
  [../]
  [./eta0]
  [../]
[]

[ICs]

  [./IC_eta2]
    x1 = 0
    y1 = 15
    x2 = 30
    y2 = 30
    inside = 1.0
    outside = 0.0
    type = BoundingBoxIC
    variable = eta2
    int_width = 0
  [../]
  [./IC_eta3]
    x1 = 15
    y1 = 0
    x2 = 30
    y2 = 15
    inside = 1.0
    outside = 0.0
    type = BoundingBoxIC
    variable = eta3
    int_width = 0
  [../]
  [./IC_eta4]
    x1 = 0
    y1 = 0
    x2 = 15
    y2 = 15
    inside = 1.0
    outside = 0.0
    type = BoundingBoxIC
    variable = eta0
    int_width = 0
  [../]
  [./IC_c]
    x1 = 15
    y1 = 15
    radius = 8.0
    outvalue = 0.05
    variable = c
    invalue = 1.0
    type = SmoothCircleIC
    int_width = 3.0
  [../]
  [./IC_eta1]
    x1 = 15
    y1 = 15
    radius = 8.0
    outvalue = 0.0
    variable = eta1
    invalue = 1.0
    type = SmoothCircleIC
    int_width = 3.0
  [../]
[]

# Not evalulating time evolution to improve test performance, since we are only testing
# the material property.

[Materials]
  [./ga_test]
    type = SwitchingFunctionMultiPhaseKundinMaterial
    h_name = ga
    eta_i = eta1
    all_etas = 'eta0 eta1 eta2 eta3'
    outputs = exodus
  [../]
  [./gb_test]
    type = SwitchingFunctionMultiPhaseKundinMaterial
    h_name = gb
    eta_i = eta0
    all_etas = 'eta0 eta1 eta2 eta3'
    outputs = exodus
  [../]
  [./const]
    type = GenericConstantMaterial
    prop_names = 'kappa_c kappa_s kappa_op L M'
    prop_values = '0 3 3 1.0 1.0'
    outputs = exodus
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  exodus = true
[]
