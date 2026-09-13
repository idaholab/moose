[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    elem_type = HEX8
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
  # gamma_1 (xi*eta) at nodes corresponds to f = (1-2x)(1-2y); the spec
  # overrides this function for the other three modes.
  [u]
    type = FunctionIC
    variable = disp_x
    function = '(1-2*x)*(1-2*y)'
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
  # For a pure mode with amplitude 1 on the unit cube: A = 2 I, h^2 = 2, V = 1,
  # c = 1/2, H = 8, nodal residuals +/-4 -> norm sqrt(8*16) = 11.313708498985
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
