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
# the material property. However, the kernel and free energy are left in place to allow
# this test to be easily turned in to a working example

#[Kernels]
#  [./c_dot]
#    type = CoupledTimeDerivative
#    variable = w
#    v = c
#  [../]
#  [./c_res]
#    type = SplitCHParsed
#    variable = c
#    f_name = F
#    kappa_name = kappa_c
#    w = w
#    coupled_variables = 'eta1 eta2 eta3 eta0'
#  [../]
#  [./w_res]
#    # coupled_variables = 'c'
#    type = SplitCHWRes
#    variable = w
#    mob_name = M
#  [../]
#  [./AC1_bulk]
#    type = AllenCahn
#    variable = eta1
#    f_name = F
#    coupled_variables = 'c eta2 eta3 eta0'
#  [../]
#  [./AC1_int]
#    type = ACInterface
#    variable = eta1
#    kappa_name = kappa_s
#  [../]
#  [./e1_dot]
#    type = TimeDerivative
#    variable = eta1
#  [../]
#  [./AC2_bulk]
#    type = AllenCahn
#    variable = eta2
#    f_name = F
#    coupled_variables = 'c eta1 eta3 eta0'
#  [../]
#  [./AC2_int]
#    type = ACInterface
#    variable = eta2
#  [../]
#  [./e2_dot]
#    type = TimeDerivative
#    variable = eta2
#  [../]
#  [./AC3_bulk]
#    type = AllenCahn
#    variable = eta3
#    f_name = F
#    coupled_variables = 'c eta2 eta1 eta0'
#  [../]
#  [./AC3_int]
#    type = ACInterface
#    variable = eta3
#  [../]
#  [./e3_dot]
#    type = TimeDerivative
#    variable = eta3
#  [../]
#  [./AC4_bulk]
#    type = AllenCahn
#    variable = eta0
#    f_name = F
#    coupled_variables = 'c eta2 eta3 eta1'
#  [../]
#  [./AC4_int]
#    type = ACInterface
#    variable = eta0
#  [../]
#  [./e4_dot]
#    type = TimeDerivative
#    variable = eta0
#  [../]
#[]

[Materials]
  [./ha_test]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = ha
    all_etas = 'eta0 eta1 eta2 eta3'
    phase_etas = 'eta1'
    outputs = exodus
  [../]
  [./hb_test]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = hb
    all_etas = 'eta0 eta1 eta2 eta3'
    phase_etas = 'eta0 eta2 eta3'
    outputs = exodus
  [../]
  #[./ha]
  #  type = DerivativeParsedMaterial
  #  coupled_variables = 'eta1 eta2 eta3 eta0'
  #  property_name = ha_parsed
  #  expression = 'eta1^2/(eta1^2+eta2^2+eta3^2+eta0^2)'
  #  derivative_order = 2
  #  outputs = exodus
  #[../]
  #[./hb]
  #  type = DerivativeParsedMaterial
  #  coupled_variables = 'eta1 eta2 eta3 eta0'
  #  property_name = hb_parsed
  #  expression = '(eta2^2+eta3^2+eta0^2)/(eta1^2+eta2^2+eta3^2+eta0^2)'
  #  derivative_order = 2
  #  outputs = exodus
  #[../]

  #[./FreeEng]
  #  type = DerivativeParsedMaterial
  #  coupled_variables = 'c eta1 eta2 eta3 eta0'
  #  property_name = F
  #  constant_names = 'c1 c2 s g d e h z'
  #  constant_expressions = '1.0 0.0 1.5 1.5 1.0 1.0 1 1.0'
  #  material_property_names = 'ha(eta1,eta2,eta3,eta0) hb(eta1,eta2,eta3,eta0)'
  #  expression = 'a:=eta1^2/(eta1^2+eta2^2+eta3^2+eta0^2);f1:=ha*(c-c1)^2;b:=(eta2^2+eta3^2+eta0^2)/(eta1^2+eta2^2+eta3^2+eta0^2);f2:=hb*(c-c2)^2
  #  ;f3:=1/4*eta1^4-1/2*eta1^2+1/4*eta2^4-1/2*eta2^2+1/4*eta3^4-1/2*eta3^2+1/4*eta0^4-1/2*eta0^2
  #  ;f4:=z*s*(eta1^2*eta2^2+eta1^2*eta3^2+eta1^2*eta0^2)+g*(eta2^2*eta3^2+eta2^2*eta0^2+eta3^2*eta0^2);f:=1/4+e*f1+d*f2+h*(f3+f4);f'
  #  derivative_order = 2
  #[../]
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
