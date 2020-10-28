
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 20
  xmax = 1
  ymax = 2
[]
[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
[]

[DiracKernels]
  [vpp_point_source]
    type = VectorPostprocessorPointSource
    variable = temperature
    vector_postprocessor = receive_values_sub
    value_name = value
  []
  [vpp_point_source]
    type = VectorPostprocessorPointSource
    variable = temperature
    vector_postprocessor = receive_values_sub
    value_name = value
  []
[]

# all objective func parameters be controolable in sub-app
# changing material properties will need to be controllable
# full postprocessors that get the measurement values back.
# Mapping for objectiving function parameters and what it controls in sub
# and mapping from post proc value in the subapp and how it corresponds to a measurement value (target value)

# list of controllable parameters
#list of postprocessors that comput the objective


[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 0
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Executioner]
  type = Steady
[]

[VectorPostprocessors]
  [./sample_points_sub]
    type = PointValueSampler
    variable = temperature
    points = '0.25 1.25 0 0.5 1.5 0'
    sort_by = id
    execute_on = 'initial timestep_end'
  [../]
  [./receive_values_sub]
    type = VectorPostprocessorReceiver
    execute_on = 'timestep_begin'
  [../]
[]

[Outputs]
  console = true
  file_base = 'zmaster/sub'
[]
