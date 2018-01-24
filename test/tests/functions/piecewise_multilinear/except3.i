# PiecewiseMultilinear function exception test
# Incorrect number of data points

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
    function = except3_fcn
  [../]
[]


[Functions]
  [./except3_fcn]
    type = PiecewiseMultilinear
    data_file = except3.txt
  [../]
[]


[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  hide = dummy
[]
