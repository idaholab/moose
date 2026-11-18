# Robin boundary condition of an array variable on a displaced mesh whose displacements are
# nonlinear variables, so the face measure depends on them. With use_displaced_mesh = true the
# Jacobian includes the derivatives of the face measure with respect to the displacements.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  displacements = 'disp_x disp_y'
[]

[Variables]
  [u]
    components = 2
  []
  [disp_x]
  []
  [disp_y]
  []
[]

[ICs]
  [disp_x]
    type = FunctionIC
    variable = disp_x
    function = '0.1*x*y'
  []
  [disp_y]
    type = FunctionIC
    variable = disp_y
    function = '0.2*x*y'
  []
[]

[Kernels]
  [diff]
    type = ADArrayDiffusion
    variable = u
  []
  [disp_x]
    type = ADDiffusion
    variable = disp_x
  []
  [disp_y]
    type = ADDiffusion
    variable = disp_y
  []
[]

[BCs]
  [left]
    type = ADArrayDirichletBC
    variable = u
    boundary = left
    values = '1 2'
  []
  [right]
    type = ADArrayRobinBC
    variable = u
    boundary = right
    value = '0.5 -1'
    alpha = '2 1'
    use_displaced_mesh = true
  []
  [disp_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [disp_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = left
    value = 0
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
