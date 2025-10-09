[Mesh]
  # Single QUAD4 element used to exercise hourglass correction.
  # This input initializes displacement fields in patterns that excite
  # the classical hourglass measures on a reduced-integration Quad4.
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    elem_type = QUAD4
  []
[]

[Variables]
  # Two scalar displacement components (2D). Initial conditions below
  # are chosen to create a non-affine field on the single element.
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      # Bilinear xy content relative to center; excites non-affine content.
      function = '(y-0.5) * (x-0.5)'
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
  # Hourglass correction kernels per component. For pure demonstration of
  # hourglass behavior, use constant one-point quadrature in the Executioner block.
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
  # Sample nodal values to observe how the hourglass correction
  # influences the final steady-state solution.
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
