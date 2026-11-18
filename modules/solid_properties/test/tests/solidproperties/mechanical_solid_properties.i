# Parameterized input template for testing MechanicalSolidProperties
# This input file is used by multiple tests with different materials via CLI parameters

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
  nx = 100  # 100 temperature points across range
[]

[SolidProperties]
  [sp]
    type = ${solid_properties_class}
  []
[]

[Materials]
  [sp_mat]
    type = MechanicalSolidPropertiesMaterial
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
    property = 'T E nu alpha'
    sort_by = x
  []
[]

[Problem]
  solve = false  # No solve needed, just evaluate properties
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = ${file_base}
  csv = true
[]
