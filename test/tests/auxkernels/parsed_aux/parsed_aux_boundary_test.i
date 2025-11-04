[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 5
  ny = 3
  allow_renumbering = false
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []

  [v]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[AuxVariables]
  [boundary_values_fe_qp]
    order = CONSTANT
    family = MONOMIAL
  []
  [boundary_values_fe_noqp]
    order = CONSTANT
    family = MONOMIAL
  []
  [boundary_values_fv_qp]
    order = CONSTANT
    family = MONOMIAL
  []
  [boundary_values_fv_noqp]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [react]
    type = BodyForce
    variable = u
    # trigger some boundary-tangential variation
    function = 'x*x + y'
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []

  [right_u]
    type = NeumannBC
    variable = u
    boundary = 'right'
    value = 1
  []
[]

[FVKernels]
  [diff_v]
    type = FVDiffusion
    variable = v
    coeff = 1
  []
  [react_v]
    type = FVBodyForce
    variable = v
    function = 'x*x + y'
  []
[]

[FVBCs]
  [left_v]
    type = FVDirichletBC
    variable = v
    boundary = 'left'
    value = '0'
  []

  [right_v]
    type = FVNeumannBC
    variable = v
    boundary = 'right'
    value = 10
  []
[]

[AuxKernels]
  [boundary_values_fe_qp]
    type = ParsedAux
    variable = boundary_values_fe_qp
    expression = u
    functor_names = u
    boundary = 'left right'
  []
  [boundary_values_fe_noqp]
    type = ParsedAux
    variable = boundary_values_fe_noqp
    expression = u
    functor_names = u
    evaluate_functors_on_qp = false
    boundary = 'left right'
  []
  [boundary_values_fv_qp]
    type = ParsedAux
    variable = boundary_values_fv_qp
    expression = v
    functor_names = v
    boundary = 'left right'
  []
  [boundary_values_fv_noqp]
    type = ParsedAux
    variable = boundary_values_fv_noqp
    expression = v
    functor_names = v
    evaluate_functors_on_qp = false
    boundary = 'left right'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  csv = true
[]

[VectorPostprocessors]
  # on the left, the face FV argument in ParsedAux picks up the dirichlet BC,
  # while when using the ElemSideQp argument, we use a two term and miss it
  # For FE, we get the DirichletBC with both arguments
  [sampler_left]
    type = SideValueSampler
    variable = 'boundary_values_fe_qp boundary_values_fe_noqp boundary_values_fv_qp boundary_values_fv_noqp'
    boundary = 'left'
    sort_by = 'id'
  []
  [sampler_right]
    type = SideValueSampler
    variable = 'boundary_values_fe_qp boundary_values_fe_noqp boundary_values_fv_qp boundary_values_fv_noqp'
    boundary = 'right'
    sort_by = 'id'
  []
[]
