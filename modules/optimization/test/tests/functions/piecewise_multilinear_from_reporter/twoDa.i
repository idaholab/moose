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

[Reporters]
  [gridData]
    type = GriddedDataReporter
    data_file = 'twoD1.txt'
    outputs = none
  []
  [exception]
    type = ConstantReporter
    real_vector_names = tooManyParams
    real_vector_values = '1 2 3 4 5 6 7 8 9 10'
  []
[]

[Functions]
# This is just f = 1 + 2x + 3y
  [bilinear1_fcn]
    type = PiecewiseMultilinearFromReporter
    values_name = 'gridData/parameter'
    grid_name = 'gridData/grid'
    axes_name = 'gridData/axes'
    step_name = 'gridData/step'
    dim_name = 'gridData/dim'
  []
  [bilinear1_answer]
    type = ParsedFunction
    value = 1+2*x+3*y
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
  exodus=true
[]
