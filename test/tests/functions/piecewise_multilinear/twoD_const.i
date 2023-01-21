# PiecewiseMultilinear function tests in 2D
# See [Functions] block for a description of the tests
# The functions are compared with ParsedFunctions using postprocessors

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 2
  nx = 4
  ymin = -1
  ymax =  1
  ny = 4
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
  [./constant]
    family = MONOMIAL
    order = CONSTANT
  [../]

  [./constant_ref]
    family = MONOMIAL
    order = CONSTANT
  [../]

  [./diff]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./const_AuxK]
    type = FunctionAux
    variable = constant
    function = const_fcn
  [../]

  [./const_ref_AuxK]
    type = FunctionAux
    variable = constant_ref
    function = const_ref
  [../]

  [./diff]
    type = ParsedAux
    variable = diff
    expression = 'constant - constant_ref'
    coupled_variables = 'constant constant_ref'
  [../]
[]


[Functions]
  [./const_fcn]
    type = PiecewiseMulticonstant
    direction = 'left right'
    data_file = twoD_const.txt
  [../]

  [./const_ref]
    type = ParsedFunction
    expression = '
            ix := if(x < 0.5, 0, if(x < 1, 1, 2));
            iy := if(y > 0, 2, if(y > -0.5, 1, 0));
            iy * 3 + ix
            '
  [../]
[]


[Postprocessors]
  [./diff_pp]
    type = ElementIntegralVariablePostprocessor
    variable = diff
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = twoD_const
  hide = dummy
  exodus = true
[]
