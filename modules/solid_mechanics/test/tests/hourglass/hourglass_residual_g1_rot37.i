[Mesh]
  # Single QUAD4 element rotated by 37 degrees; IC uses inverse-rotated coords to recreate g1.
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    elem_type = QUAD4
  []
  [rot]
    type = TransformGenerator
    input = gen
    transform = ROTATE
    vector_value = '0 0 37'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[ICs]
  # Undo-rotate coordinates by -37 deg: x' = c*x + s*y, y' = -s*x + c*y
  # g1(u) = 1 - 2*x' - 2*y' + 4*x'*y'
  [u]
    type = FunctionIC
    variable = disp_x
    function = 'a:=37/180*pi; s:=sin(a); c:=cos(a); 1 - 2*(c*x + s*y) - 2*(-s*x + c*y) + 4*(c*x + s*y)*(-s*x + c*y)'
  []
  [v]
    type = FunctionIC
    variable = disp_y
    function = 0
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

