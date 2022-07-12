# This is changed by main.i for testing purposes
real_val = 0.0
vector_val0 = ${fparse real_val * 10}
vector_val1= ${fparse vector_val0 * 10}
vector_val2= ${fparse vector_val0 * 100}
vector_val3= ${fparse vector_val0 * 1000}


[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = ${real_val}
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.01
  dtmin = 0.01
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  error_on_dtmin = false
[]


[Postprocessors]
  [pp]
    type = PointValue
    point = '0 0 0'
    variable = u
  []
[]

[VectorPostprocessors]
  [vpp]
    type = ConstantVectorPostprocessor
    vector_names = 'vec'
    value = '${vector_val0} ${vector_val1} ${vector_val2} ${vector_val3}'
  []
[]

[Reporters]
  [constant]
    type = ConstantReporter
    integer_names = 'int'
    integer_values = 0
    string_names = 'str'
    string_values = 'this_value'
  []
  [mesh]
    type = MeshInfo
    items = sidesets
  []
[]

# This is used in main_batch.i
[Controls]
  [stm]
    type = SamplerReceiver
  []
[]
