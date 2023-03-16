[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[Problem]
  solve = false
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
    prop_names = 'rho vel k mu cp T T_wall D_h'
    prop_values = '1000 0.1 0.001 0.1 12 300 310 0.1'
  []

  [Hw_material]
    type = ADWallHeatTransferCoefficientWolfMcCarthyMaterial
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
  execute_on = timestep_end
[]
