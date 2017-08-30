# PiecewiseMultilinear function exception test
# Grid is not monotonic

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
    data_file = except2.txt
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
