# PiecewiseMultilinear function tests for time-dependent data
# See [Functions] block for a description of the tests

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 1
  nx = 1
  ymin = 0
  ymax = 1
  ny = 1
  zmin = 0
  zmax = 1
  nz = 1
[]

[Variables]
  [./dummy]
  [../]
[]

[Kernels]
  [./dummy_u]
    type = TimeDerivative
    variable = dummy
  [../]
[]


[AuxVariables]
  [./time1_var]
  [../]
[]

[AuxKernels]
  [./time1_AuxK]
    type = FunctionAux
    variable = time1_var
    function = time1_fcn
  [../]
[]

[Functions]
# This increases linearly: f = t
  [./time1_fcn]
    type = PiecewiseMultilinear
    data_file = time1.txt
  [../]
  [./time1_answer]
    type = ParsedFunction
    expression = t
  [../]
[]

[Postprocessors]
  [./time1_pp]
    type = NodalL2Error
    function = time1_answer
    variable = time1_var
  [../]
[]


[Executioner]
  type = Transient
  dt = 0.1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = time
  hide = dummy
  csv = true
[]
