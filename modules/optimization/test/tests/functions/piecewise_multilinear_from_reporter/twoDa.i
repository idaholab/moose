# PiecewiseMultilinear function tests in 2D
# See [Functions] block for a description of the tests
# The functions are compared with ParsedFunctions using postprocessors

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  nx = 6
  ymin = 0
  ymax = 1
  ny = 6
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
  [./bilinear1_var]
  [../]
[]

[AuxKernels]
  [./bilinear1_AuxK]
    type = FunctionAux
    variable = bilinear1_var
    function = bilinear1_fcn
  [../]
[]


[Functions]
# This is just f = 1 + 2x + 3y
  [./bilinear1_fcn]
    type = PiecewiseMultilinearFromReporter
  value_name = 'reporterData1/u'
  x_coord_name = 'reporterData1/x'
  y_coord_name = 'reporterData1/y'
  z_coord_name = 'reporterData1/z'
  time_name = 'reporterData1/z'
[]

  [./bilinear1_answer]
    type = ParsedFunction
    value = 1+2*x+3*y
  [../]
[]
[Reporters]
  [reporterData1]
  type = ConstantReporter
  real_vector_names = 'x y z t u'
  real_vector_values = '-1.0 0.0 2.0; -1.0 2.0 3.0; 0; 0; -4.0 -2.0 2.0 5.0 7.0 11.0 8.0 10.0 14.0'
  []
[]
[Postprocessors]
  [./bilinear1_pp]
    type = NodalL2Error
    function = bilinear1_answer
    variable = bilinear1_var
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = twoDa
  hide = dummy
  csv = true
[]
