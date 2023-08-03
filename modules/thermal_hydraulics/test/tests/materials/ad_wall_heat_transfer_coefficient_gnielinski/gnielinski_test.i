#Helium properties at 7.0 MPa and 1073 K
rho = 3.1176
vel = 1
k = 0.38220
mu = 4.8587e-05
cp = 5189.8
T = 1073
T_wall = 1074
D_h = 1

[GlobalParams]
  execute_on = 'initial'
[]

[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[AuxVariables]
  [Hw]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [Hw_ak]
    type = ADMaterialRealAux
    variable = Hw
    property = Hw
  []
[]

[Materials]
  [props]
    type = ADGenericConstantMaterial
    prop_names = 'rho  vel  k  mu  cp  T  T_wall  D_h'
    prop_values = '${rho}  ${vel}  ${k}  ${mu}  ${cp}  ${T}  ${T_wall}  ${D_h}'
  []

  [Hw_material]
    type = ADWallHeatTransferCoefficientGnielinskiMaterial
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [Hw]
    type = ElementalVariableValue
    elementid = 0
    variable = Hw
  []
[]

[Outputs]
  csv = true
[]

