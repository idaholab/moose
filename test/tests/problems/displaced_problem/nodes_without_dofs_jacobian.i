# AD kernels and boundary conditions on the displaced mesh depend on the positions of the TET14
# face nodes, which carry no SECOND order displacement degrees of freedom and move with the
# displacement of the other element nodes.
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = TET4
  []
  [complete]
    type = ElementOrderConversionGenerator
    input = gmg
    conversion_type = COMPLETE_ORDER
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [u]
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = 'x * x + y * z'
    []
  []
  [disp_x]
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = '0.1 * y * z'
    []
  []
  [disp_y]
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = '0.1 * x * z'
    []
  []
  [disp_z]
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = '0.1 * x * (x + z)'
    []
  []
[]

[Kernels]
  [diff_u]
    type = ADDiffusion
    variable = u
    use_displaced_mesh = true
  []
  [diff_x]
    type = ADDiffusion
    variable = disp_x
  []
  [diff_y]
    type = ADDiffusion
    variable = disp_y
  []
  [diff_z]
    type = ADDiffusion
    variable = disp_z
  []
[]

[BCs]
  [flux]
    type = ADNeumannBC
    variable = u
    boundary = 'right top'
    value = 1
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
