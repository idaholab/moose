# This input file is designed to test adding extra stress to ADComputeLinearElasticStress
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 50
  ymax = 50
[]

[AuxVariables]
  [vonmises_stress]
    order = FIRST
    family = LAGRANGE
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Modules/TensorMechanics/Master/All]
  strain = SMALL
  add_variables = true
  generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx hydrostatic_stress'
  material_output_order = 'CONSTANT CONSTANT CONSTANT CONSTANT CONSTANT CONSTANT CONSTANT'
  material_output_family = 'MONOMIAL MONOMIAL MONOMIAL MONOMIAL MONOMIAL MONOMIAL MONOMIAL'
  use_automatic_differentiation = true
[]

[AuxKernels]
  [vonmises_stress]
    type = ADRankTwoScalarAux
    variable = vonmises_stress
    rank_two_tensor = stress
    scalar_type = VonMisesStress
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0
  []
  [stress]
    type = ADComputeLinearElasticStress
    extra_stress_names = 'stress_one stress_two'
  []
  [stress_one]
    type = GenericConstantRankTwoTensor
    tensor_name = stress_one
    tensor_values = '0 1e3 1e3 1e3 0 1e3 1e3 1e3 0'
  []
  [stress_two]
    type = GenericConstantRankTwoTensor
    tensor_name = stress_two
    tensor_values = '1e3 0 0 0 1e3 0 0 0 1e3'
  []
[]

[BCs]
  [disp_x_BC]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'bottom top'
    value = 0.5
  []
  [disp_x_BC2]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'left right'
    value = 0.01
  []
  [disp_y_BC]
    type = ADDirichletBC
    variable = disp_y
    boundary = 'bottom top'
    value = 0.8
  []
  [disp_y_BC2]
    type = ADDirichletBC
    variable = disp_y
    boundary = 'left right'
    value = 0.02
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Postprocessors]
  [hydrostatic]
    type = ElementAverageValue
    variable = hydrostatic_stress
  []
  [von_mises]
    type = ElementAverageValue
    variable = vonmises_stress
  []
[]

[Outputs]
  exodus = true
[]
