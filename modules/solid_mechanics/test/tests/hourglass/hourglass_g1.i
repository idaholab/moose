[Mesh]
  # Single QUAD4 element; initialize displacement in the g1 hourglass pattern
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
      # g1 at nodes [1,-1,1,-1] is represented by f(x,y) = 1 - 2x - 2y + 4xy
      function = '1 - 2*x - 2*y + 4*x*y'
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
  # Use the advanced hourglass kernel; set mu=1 for simplicity here
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
[]

[Executioner]
  type = Steady

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

