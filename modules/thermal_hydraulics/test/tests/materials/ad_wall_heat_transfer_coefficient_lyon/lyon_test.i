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
    #liquid sodium properties at 773 K
    type = ADGenericConstantMaterial
    prop_names = ' rho       vel    k          mu        cp     T   T_wall D_h'
    prop_values = '762.90    0.1    64.217     2.358e-4  1264.6 773 774    0.1'
  []

  [Hw_material]
    type = ADWallHeatTransferCoefficientLyonMaterial
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
