[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    elem_type = HEX8
  []
  [rot]
    type = TransformGenerator
    input = gen
    transform = ROTATE
    vector_value = '37 21 0'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = disp_x
    function = '0.1 + 0.02*x - 0.03*y + 0.05*z'
  []
  [v]
    type = FunctionIC
    variable = disp_y
    function = '-0.2 + 0.07*x + 0.01*y - 0.04*z'
  []
  [w]
    type = FunctionIC
    variable = disp_z
    function = '0.03 - 0.06*x + 0.02*y + 0.08*z'
  []
[]

[Kernels]
  [hourglass_x]
    type = HourglassCorrectionHex8
    variable = disp_x
    penalty = 1
    shear_modulus = 1
  []
  [hourglass_y]
    type = HourglassCorrectionHex8
    variable = disp_y
    penalty = 1
    shear_modulus = 1
  []
  [hourglass_z]
    type = HourglassCorrectionHex8
    variable = disp_z
    penalty = 1
    shear_modulus = 1
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
  [Quadrature]
    type = GAUSS
    order = CONSTANT
  []
[]

[Postprocessors]
  [hg_res]
    type = Residual
    residual_type = COMPUTE
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
