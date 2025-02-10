[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    elem_type = QUAD4
  []
[]

[Variables]
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      function = '(y-0.5) * (x-0.5)'
      # function = x
    []
  []
  [disp_y]
    [InitialCondition]
      type = FunctionIC
      function = 0 # y
    []
  []
[]

[Kernels]
  [hourglass_x]
    type = HourglassCorrectionQuad4
    variable = disp_x
    penalty = 1
  []
  [hourglass_y]
    type = HourglassCorrectionQuad4
    variable = disp_y
    penalty = 1
  []
[]

[Problem]
  kernel_coverage_check = FALSE
[]

[Executioner]
  type = Steady

  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = ' lu       NONZERO               1e-10'

  [Quadrature]
    type = GAUSS
    order = CONSTANT
  []
[]

[VectorPostprocessors]
  [nodes]
    type = NodalValueSampler
    sort_by = X
    variable = 'disp_x disp_y'
    execute_on = 'INITIAL FINAL'
  []
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL FINAL'
[]
