#liquid sodium properties at 773 K
rho = 762.90
vel = 0.1
k = 64.217
mu = 2.358e-4
cp = 1264.6
T = 773
T_wall = 774
D_h = 0.1
PoD = 1.1

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
    type = ADWallHeatTransferCoefficientKazimiMaterial
    PoD = ${PoD}
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

