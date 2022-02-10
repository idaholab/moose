[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
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
    prop_names = 'Nu k D_h'
    prop_values = '1000 2 20'
  []

  [Hw_material]
    type = ADConvectiveHeatTransferCoefficientMaterial
    Nu = Nu
    D_h = D_h
    k = k
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
