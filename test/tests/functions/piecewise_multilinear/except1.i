# PiecewiseMultilinear function exception test
# Data file does not exist

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
    function = except1_fcn
  [../]
[]


[Functions]
  [./except1_fcn]
    type = PiecewiseMultilinear
    data_file = except1.txt
  [../]
[]


[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = except1
  hide = dummy
  csv = true
[]
