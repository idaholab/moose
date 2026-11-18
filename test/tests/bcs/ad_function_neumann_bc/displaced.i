# The boundary condition function depends on position, and the quadrature points on the displaced
# mesh move with the displacements, so the AD Jacobian of the boundary condition has derivatives
# with respect to the displacements.
[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [u]
  []
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      function = '0.1 * x * y'
    []
  []
  [disp_y]
    [InitialCondition]
      type = FunctionIC
      function = '0.1 * x * x'
    []
  []
[]

[Kernels]
  [diff_u]
    type = ADDiffusion
    variable = u
  []
  [diff_x]
    type = ADDiffusion
    variable = disp_x
  []
  [diff_y]
    type = ADDiffusion
    variable = disp_y
  []
[]

[BCs]
  [right]
    type = ADFunctionNeumannBC
    variable = u
    boundary = 'right top'
    function = '1 + x + y'
    use_displaced_mesh = true
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]
