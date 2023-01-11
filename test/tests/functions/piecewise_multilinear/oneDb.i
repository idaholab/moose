# PiecewiseMultilinear function tests in 1D
# See [Functions] block for a description of the tests
# The functions are compared with ParsedFunctions using postprocessors

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 2
  nx = 10
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
  [./linear1_var]
  [../]
  [./linear2_var]
  [../]
[]

[AuxKernels]
  [./linear1_AuxK]
    type = FunctionAux
    variable = linear1_var
    function = linear1_fcn
  [../]
  [./linear2_AuxK]
    type = FunctionAux
    variable = linear2_var
    function = linear2_fcn
  [../]
[]


[Functions]
# This is just f = x
  [./linear1_fcn]
    type = PiecewiseMultilinear
    data_file = linear1.txt
  [../]
  [./linear1_answer]
    type = ParsedFunction
    expression = x
  [../]

# This is a hat function
  [./linear2_fcn]
    type = PiecewiseMultilinear
    data_file = linear2.txt
  [../]
  [./linear2_answer]
    type = ParsedFunction
    expression = min(x,1)+min(2-x,1)-1
  [../]

[]

[Postprocessors]
  [./linear1_pp]
    type = NodalL2Error
    function = linear1_answer
    variable = linear1_var
  [../]
  [./linear2_pp]
    type = NodalL2Error
    function = linear2_answer
    variable = linear2_var
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = oneDb
  hide = dummy
  csv = true
[]
