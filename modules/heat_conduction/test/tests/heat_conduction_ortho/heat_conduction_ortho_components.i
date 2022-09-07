# The temperature is set at 273K and the thermal conductivity components
# are computed as functions of temperature.
# k_xx = (273 - 173) * 1.0 + 10.0 = 110.0
# k_yy = (273 - 173) * 2.0 + 20.0 = 220.0
# k_zz = (273 - 173) * 1.0 + 30.0 = 330.0
# Same values of thermal conductivity components are calculated by Moose.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[Variables]
  [temperature]
    initial_condition = 273.0
  []
[]

[AuxVariables]
  [thermal_conductivity_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [thermal_conductivity_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [thermal_conductivity_z]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Functions]
  [k_xx]
    type = ParsedFunction
    value = '(t-173.0)*1.0+10.0' # t is used as temperature
  []
  [k_yy]
    type = ParsedFunction
    value = '(t-173.0)*2.0+20.0' # t is used as temperature
  []
  [k_zz]
    type = ParsedFunction
    value = '(t-173.0)*3.0+30.0' # t is used as temperature
  []
[]

[Kernels]
  [heat]
    type = AnisoHeatConduction
    variable = temperature
  []
[]

[AuxKernels]
  [thermal_conductivity_x]
    type = MaterialRealTensorValueAux
    variable = thermal_conductivity_x
    property = thermal_conductivity
    execute_on = timestep_end
  []
  [thermal_conductivity_y]
    type = MaterialRealTensorValueAux
    variable = thermal_conductivity_y
    property = thermal_conductivity
    row = 1
    column = 1
    execute_on = timestep_end
  []
  [thermal_conductivity_z]
    type = MaterialRealTensorValueAux
    variable = thermal_conductivity_z
    property = thermal_conductivity
    row = 2
    column = 2
    execute_on = timestep_end
  []
[]

[BCs]
  [temperatures]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 273.0
  []
  [neum]
    type = NeumannBC
    variable = temperature
    boundary = right
    value = 0.0
  []
[]

[Materials]
  [heat]
    type = AnisoHeatConductionMaterial
    specific_heat = 0.116
    k_11 = k_xx
    k_22 = k_yy
    k_33 = k_zz
    temperature = temperature
  []
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = 0.283
  []
[]

[Executioner]
  type = Steady

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'
  line_search = 'none'

  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-10
  l_max_its = 20
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [thermal_conductivity_x]
    type = ElementAverageValue
    variable = thermal_conductivity_x
    execute_on = 'initial timestep_end'
  []
  [thermal_conductivity_y]
    type = ElementAverageValue
    variable = thermal_conductivity_y
    execute_on = 'initial timestep_end'
  []
  [thermal_conductivity_z]
    type = ElementAverageValue
    variable = thermal_conductivity_z
    execute_on = 'initial timestep_end'
  []
[]
