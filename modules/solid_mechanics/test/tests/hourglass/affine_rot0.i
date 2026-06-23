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
  []
  [disp_y]
  []
[]

[Kernels]
  [hg_x]
    type = HourglassCorrectionQuad4
    variable = disp_x
    penalty = 1
    shear_modulus = 1
  []
  [hg_y]
    type = HourglassCorrectionQuad4
    variable = disp_y
    penalty = 1
    shear_modulus = 1
  []
[]

[ICs]
  # Impose a purely affine displacement field u = ax + by + c, v = dx + ey + f
  [u]
    type = FunctionIC
    variable = disp_x
    function = '0.1*x + 0.2*y + 0.05'
  []
  [v]
    type = FunctionIC
    variable = disp_y
    function = '-0.1*x + 0.3*y + 0.0'
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

