# This input file is used for testing an arbitrary "ThermalSolidProperties"
# user object. Density, specific heat capacity, and thermal conductivity are
# computed over a range of temperature values.

solid_properties_class = placeholder
file_base = placeholder
T_min = placeholder
T_max = placeholder

[GlobalParams]
  execute_on = 'INITIAL'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 100
[]

[SolidProperties]
  [sp]
    type = ${solid_properties_class}
  []
[]

[Materials]
  [sp_mat]
    type = ThermalSolidPropertiesMaterial
    temperature = T
    sp = sp
  []
  [T_mat]
    type = GenericFunctionMaterial
    prop_names = 'T'
    prop_values = 'T_fn'
  []
[]

[Functions]
  [T_fn]
    type = PiecewiseLinear
    axis = x
    x = '0 1'
    y = '${T_min} ${T_max}'
  []
[]

[AuxVariables]
  [T]
  []
[]

[AuxKernels]
  [T_ak]
    type = FunctionAux
    variable = T
    function = T_fn
    execute_on = 'INITIAL'
  []
[]

[VectorPostprocessors]
  [vpp]
    type = LineMaterialRealSampler
    start = '0 0 0'
    end = '1 0 0'
    property = 'T density specific_heat thermal_conductivity'
    sort_by = x
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = ${file_base}
  csv = true
[]
