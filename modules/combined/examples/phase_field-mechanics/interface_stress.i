[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 50
  ny = 50
  nz = 50
  xmax = 10
  ymax = 10
  zmax = 10
  xmin = -10
  ymin = -10
  zmin = -10
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./sphere]
    type = ParsedFunction
    expression = 'r:=sqrt(x^2+y^2+z^2); R:=(4.0-r)/2.0; if(R>1,1,if(R<0,0,3*R^2-2*R^3))'
  [../]
[]

[AuxVariables]
  [./eta]
    [./InitialCondition]
      type = FunctionIC
      function = sphere
    [../]
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    add_variables = true
    generate_output = 'hydrostatic_stress stress_xx'
  [../]
[]

[Materials]
  [./ym]
    type = DerivativeParsedMaterial
    property_name = ym
    expression = (1-eta)*7+0.5
    coupled_variables = eta
  [../]
  [./elasticity]
    type = ComputeVariableIsotropicElasticityTensor
    poissons_ratio = 0.45
    youngs_modulus = ym
    args = eta
  [../]

  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./interface]
    type = ComputeInterfaceStress
    v = eta
    stress = 1.0
  [../]
[]

[VectorPostprocessors]
  [./line]
    type = SphericalAverage
    variable = 'hydrostatic_stress'
    radius = 10
    bin_number = 40
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  csv = true
[]
