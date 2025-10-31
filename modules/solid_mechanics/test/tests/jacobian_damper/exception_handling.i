[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  xmax = 5
  ymax = 5
  zmax = 5
  elem_type = HEX8
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [tdisp]
    type = ParsedFunction
    expression = '-4 * t'
  []
[]

[AuxVariables]
  [pid]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [pid]
    type = ProcessorIDAux
    variable = pid
    execute_on = 'initial'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [time_x]
    type = TimeDerivative
    variable = disp_x
  []
  [time_y]
    type = TimeDerivative
    variable = disp_y
  []
  [time_z]
    type = TimeDerivative
    variable = disp_z
  []
  [rxn_x]
    type = Reaction
    variable = disp_x
  []
  [rxn_y]
    type = Reaction
    variable = disp_y
  []
  [rxn_z]
    type = Reaction
    variable = disp_z
  []
[]

[BCs]
  [symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [tdisp]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = tdisp
    preset = false
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Dampers]
  [jac]
    type = ElementJacobianDamper
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
  line_search = none
  dt = 1
  num_steps = 1
  nl_max_its = 5
[]
