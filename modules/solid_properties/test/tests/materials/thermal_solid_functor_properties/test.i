T_initial = 300

[GlobalParams]
  execute_on = 'INITIAL'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 1
[]

[SolidProperties]
  [ss316_sp]
    type = ThermalSS316Properties
  []
[]

[Materials]
  [sp_mat]
    type = ThermalSolidPropertiesFunctorMaterial
    temperature = T
    sp = ss316_sp
    specific_heat = cp
    density = rho
    thermal_conductivity = k
  []
[]

[AuxVariables]
  [T]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [T_ak]
    type = ConstantAux
    variable = T
    value = ${T_initial}
  []
[]

[Postprocessors]
  [cp]
    type = ElementIntegralFunctorPostprocessor
    functor = 'cp'
  []
  [k]
    type = ElementIntegralFunctorPostprocessor
    functor = 'k'
  []
  [density]
    type = ElementIntegralFunctorPostprocessor
    functor = 'rho'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
