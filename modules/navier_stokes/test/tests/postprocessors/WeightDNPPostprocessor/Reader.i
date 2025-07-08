#set mesh
[Mesh]
  type = GeneratedMesh
  allow_renumbering = false
  dim = 2
  xmin = 0
  xmax = 2
  ymin = 0
  ymax = 2
  nx = 19 
  ny = 19
[]


[Variables]
    [Dummy]
    []
[]
# PK
[AuxVariables]
  [C_tot]
  []
  [power]
  []
  [power_scaled]
  []
[]

[ICs]
  [Initial_tot]
    type = FunctionIC
    variable = 'C_tot'
    function =  C_tot_func
  []
  [Initial_Power]
    type = FunctionIC
    variable = 'power'
    function =  power_func
  []
[]

[UserObjects]
  [reader_C_tot]
    type = PropertyReadFile
    prop_file_name = 'C_tot.csv'
    read_type = 'node'
    nprop = 1  # number of columns in CSV
  []
  [reader_power]
    type = PropertyReadFile
    prop_file_name = 'power.csv'
    read_type = 'node'
    nprop = 1  # number of columns in CSV
  []
[]

[Functions]
#load from CSV
  [C_tot_func]
    type = PiecewiseConstantFromCSV
    read_prop_user_object = 'reader_C_tot'
    read_type = 'node'
    column_number = '0'
  []
  [power_func]
    type = PiecewiseConstantFromCSV
    read_prop_user_object = 'reader_power'
    read_type = 'node'
    column_number = '0'
  []
[]

[Kernels]
 [Dummy]
    type = NullKernel
    variable = Dummy
 []
[]

[AuxKernels]
    [power_scale]
        type = NormalizationAux 
        variable = power_scaled
        source_variable = power
        normalization = power_sum
    []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = false
[]

[Postprocessors]
  [power_sum]
    type = NodalSum
    execute_on = 'initial TIMESTEP_END'
    variable = power
  []
  [power_int]
    type = ElementIntegralVariablePostprocessor
    execute_on = 'initial TIMESTEP_END'
    variable = power
  []
  [power_s_sum]
    type = NodalSum
    execute_on = 'initial TIMESTEP_END'
    variable = power_scaled
  []
  [power_s_int]
    type = ElementIntegralVariablePostprocessor
    execute_on = 'initial TIMESTEP_END'
    variable = power_scaled
  []
  [C_int_end]
    type = ElementIntegralVariablePostprocessor
    execute_on = 'initial timestep_end'
    variable = C_tot
  []
[]
