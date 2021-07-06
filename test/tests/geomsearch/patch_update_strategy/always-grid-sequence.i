[Mesh]
  type = FileMesh
  file = long_range_coarse.e
  dim = 2
  patch_update_strategy = always
  displacements = 'disp_x disp_y'
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
  solution_variables = 'u'
[]

[Variables]
  [./u]
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
    extra_vector_tags = 'ref'
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
    execute_on = 'nonlinear timestep_end'
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
    type = FunctionDirichletBC
    variable = u
    boundary = 'lefttop righttop'
    function = 't'
  [../]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 'leftbottom rightbottom'
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 3
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  num_grids = 2
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [num_nl]
    type = NumNonlinearIterations
  []
  [total_nl]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
[]
