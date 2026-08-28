[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  elem_type = QUAD4
  displacements = 'disp_x disp_y'

  [Partitioner]
    type = GridPartitioner
    nx = 2
    ny = 1
    nz = 1
  []
[]

[Variables]
  [diffused]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
  [pid]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [disp_x]
    type = ParsedAux
    variable = disp_x
    expression = '0.1 * y'
    use_xyzt = true
    execute_on = INITIAL
  []
  [disp_y]
    type = ParsedAux
    variable = disp_y
    expression = '0.1 * x'
    use_xyzt = true
    execute_on = INITIAL
  []
  [pid_aux]
    type = ProcessorIDAux
    variable = pid
    execute_on = INITIAL
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = diffused
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = diffused
    preset = false
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = diffused
    preset = false
    boundary = right
    value = 0
  []
[]

[Constraints]
  [y_top]
    type = EqualValueBoundaryConstraint
    variable = diffused
    secondary = top
    primary_node_coord = '0.3333333333333 1 0'
    penalty = 10e6
    use_displaced_mesh = true
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  line_search = none
[]

[Outputs]
  execute_on = timestep_end
  exodus = true
  show = 'diffused pid'
[]
