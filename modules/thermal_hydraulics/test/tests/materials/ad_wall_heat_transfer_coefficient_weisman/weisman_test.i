#Water properties 15.1 MPa and 573 k
rho = 726.06
vel = 0.1
k = 0.56361
mu = 8.84e-05
cp = 5468.3
T = 573
T_wall = 574
D_h = 0.1
PoD = 1.1
array = "Square"
file_base = "square_cooled"

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
    type = ADWallHeatTransferCoefficientWeismanMaterial
    PoD = ${PoD}
    bundle_array = ${array}
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
  file_base = ${file_base}
[]
