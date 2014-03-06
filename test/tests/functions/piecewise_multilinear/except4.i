# PiecewiseMultilinear function exception test
# AXIS X encountered more than once

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 2
  nx = 1
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
  [./f]
  [../]
[]

[AuxKernels]
  [./f_auxK]
    type = FunctionAux
    variable = f
    function = except4_fcn
  [../]
[]


[Functions]
  [./except4_fcn]
    type = PiecewiseMultilinear
    data_file = except4.txt
  [../]


[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[Output]
  file_base = except3
  interval = 1
  hidden_variables = dummy
  exodus = false
  output_initial = false
  perf_log = true
  postprocessor_csv = false
[]
