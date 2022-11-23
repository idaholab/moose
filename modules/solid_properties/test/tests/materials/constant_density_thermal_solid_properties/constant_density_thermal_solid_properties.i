# The gold density value should reflect the reference temperature value, not
# the temperature variable value.
T_initial = 300
T_ref = 500

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
    type = ADConstantDensityThermalSolidPropertiesMaterial
    temperature = T
    sp = ss316_sp
    T_ref = ${T_ref}
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
  [density]
    type = ADElementAverageMaterialProperty
    mat_prop = density
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
