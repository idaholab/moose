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

[FunctorMaterials]
  [sp_mat]
    type = ThermalSolidPropertiesFunctorMaterial
    temperature = T
    sp = ss316_sp
    specific_heat = cp
    density = rho
    thermal_conductivity = k
    specific_internal_energy = e
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
    type = ElementExtremeFunctorValue
    functor = 'cp'
  []
  [k]
    type = ElementExtremeFunctorValue
    functor = 'k'
  []
  [density]
    type = ElementExtremeFunctorValue
    functor = 'rho'
  []
  [e]
    type = ElementExtremeFunctorValue
    functor = 'e'
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
