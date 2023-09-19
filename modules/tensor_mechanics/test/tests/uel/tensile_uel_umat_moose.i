[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    elem_type = HEX8
  []
  [extra_nodeset]
    type = ExtraNodesetGenerator
    input = mesh
    new_boundary = 'master'
    coord = '1.0 1.0 1.0'
  []
[]

# [AuxVariables]
#   [temperature]
#     initial_condition = 500
#   []
# []

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Functions]
  [function_pull]
    type = PiecewiseLinear
    x = '0 100'
    y = '0 0.1'
  []
[]

[Constraints]
  [one]
    type = LinearNodalConstraint
    variable = disp_x
    primary = '6'
    secondary_node_ids = '1 2 5'
    penalty = 1.0e8
    formulation = kinematic
    weights = '1'
  []
  [two]
    type = LinearNodalConstraint
    variable = disp_z
    primary = '6'
    secondary_node_ids = '4 5 7'
    penalty = 1.0e8
    formulation = kinematic
    weights = '1'
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
  # What's done below is to capture the weird constraints
  [axial_load]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'top'
    function = function_pull
  []
[]

[UserObjects]
  [uel]
    type = AbaqusUserElement
    variables = 'disp_x disp_y'
    plugin = '../../../../tensor_mechanics/examples/uel_build/uel'
    use_displaced_mesh = false

    #temperature = temperature # TODO
    #use_one_based_indexing = true # TODO

    jtype = 17
    num_state_vars = 177
    constant_properties = '190.0 28.0 3.0 1.0 6.0 0.0 0.0 23.0 25.0 26.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 '
                          '0.0 0.0 0.0 0.0 0.0 31700000.0 0.32 6.67e-06 1e-08 5000.0 4.0' # 27 properties
    extra_vector_tags = 'kernel_residual'
  []
[]

[Problem]
  kernel_coverage_check = false
  extra_tag_vectors = 'kernel_residual'
[]

[AuxVariables]
  [res_x]
  []
  [res_y]
  []
[]

[AuxKernels]
  [res_x]
    type = TagVectorAux
    variable = res_x
    v = disp_x
    vector_tag = kernel_residual
  []
  [res_y]
    type = TagVectorAux
    variable = res_y
    v = disp_y
    vector_tag = kernel_residual
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  l_max_its = 100
  l_tol = 1e-8
  nl_max_its = 50
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8

  dtmin = 1
  dt = 5
  end_time = 100
[]

[Outputs]
  exodus = true
[]
