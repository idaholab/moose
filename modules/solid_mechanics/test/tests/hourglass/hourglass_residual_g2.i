[Mesh]
  # Single QUAD4 element used to exercise hourglass correction with a g2 pattern.
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
      # g2 at nodes [1,1,-1,-1] is represented by f(x,y) = 1 - 2*y
      function = '1 - 2*y'
    []
  []
  [disp_y]
    [InitialCondition]
      type = FunctionIC
      function = 0
    []
  []
[]

[Kernels]
  [hourglass_x]
    type = HourglassCorrectionQuad4
    variable = disp_x
    penalty = 1
    shear_modulus = 1
  []
  [hourglass_y]
    type = HourglassCorrectionQuad4
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

