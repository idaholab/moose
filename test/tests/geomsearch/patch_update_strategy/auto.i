[Mesh]
  type = FileMesh
  file = long_range.e
  dim = 2
  patch_update_strategy = auto
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./u]
    block = right
  [../]
[]

[AuxVariables]
  [./linear_field]
  [../]
  [./receiver]
    # The field to transfer into
  [../]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./elemental_reciever]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[AuxKernels]
  [./linear_in_y]
    # This just gives us something to transfer that varies in y so we can ensure the transfer is working properly...
    type = FunctionAux
    variable = linear_field
    function = y
    execute_on = initial
  [../]
  [./right_to_left]
    type = GapValueAux
    variable = receiver
    paired_variable = linear_field
    paired_boundary = rightleft
    execute_on = timestep_end
    boundary = leftright
  [../]
  [./y_displacement]
    type = FunctionAux
    variable = disp_y
    function = t
    execute_on = 'linear timestep_begin'
    block = left
  [../]
  [./elemental_right_to_left]
    type = GapValueAux
    variable = elemental_reciever
    paired_variable = linear_field
    paired_boundary = rightleft
    boundary = leftright
  [../]
[]

[BCs]
  [./top]
    type = DirichletBC
    variable = u
    boundary = righttop
    value = 1
  [../]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = rightbottom
    value = 0
  [../]
[]

[Problem]
  type = FEProblem
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 30
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
