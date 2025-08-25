[Mesh]
  # Single QUAD4 element used to exercise hourglass correction with a g1 pattern.
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

[ICs]
  # g1 at nodes [1,-1,1,-1] corresponds to f(x,y) = 1 - 2*x - 2*y + 4*x*y
  [u]
    type = FunctionIC
    variable = disp_x
    function = '1 - 2*x - 2*y + 4*x*y'
  []
  [v]
    type = FunctionIC
    variable = disp_y
    function = 0
  []
[]


[Kernels]
  [hourglass_x]
    type = HourglassCorrectionQuad4b
    variable = disp_x
    penalty = 1
    shear_modulus = 1
  []
  [hourglass_y]
    type = HourglassCorrectionQuad4b
    variable = disp_y
    penalty = 1
    shear_modulus = 1
  []
[]

[Problem]
  kernel_coverage_check = FALSE
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
  # Compute the nonlinear residual norm at INITIAL to show it is non-zero
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

