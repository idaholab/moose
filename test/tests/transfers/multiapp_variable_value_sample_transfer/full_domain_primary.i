[Mesh]
  type = MeshGeneratorMesh

  [cartesian_basic_mesh]
    type = CartesianMeshGenerator
    dim = 2

    dx = '0.25 0.25 0.25 0.25'
    ix = '1 1 1 1 '

    dy = '0.25 0.25 0.25 0.25'
    iy = '1 1 1 1'

    subdomain_id = '1 2 2 2
                    1 1 2 2
                    1 1 2 2
                    1 1 1 2'
  []
  [central_node]
    type = ExtraNodesetGenerator
    coord = '0.5 0.5'
    input = cartesian_basic_mesh
    new_boundary = 'central_node'
  []
[]

[Variables]
  [to_subapp]
    initial_condition = -1.0
  []
[]

[AuxKernels]
  [discretize_to_subapp]
    type = ParsedAux
    variable = from_subapp_check
    expression = 'to_subapp'
    coupled_variables = 'to_subapp'
  []
  [subapp_primary_diff]
    type = ParsedAux
    variable = subapp_primary_diff
    expression = 'from_subapp_check - from_subapp'
    coupled_variables = 'from_subapp_check from_subapp'
  []
[]

[AuxVariables]
  [from_subapp]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -2.0
  []
  [from_subapp_check]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -2.0
  []
  [subapp_primary_diff]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -2.0
  []
  [array_var]
    family = MONOMIAL
    order = CONSTANT
    components = 3
    initial_condition = '-2 -1 0'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = to_subapp
  []
[]

[BCs]
  [edge]
    type = DirichletBC
    variable = to_subapp
    boundary = 'top right left bottom'
    value = 1
  []
  [center]
    type = DirichletBC
    variable = to_subapp
    boundary = 'central_node'
    value = 0
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  num_steps = 3
  dt = 1.0

  nl_abs_tol = 1e-13
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = CentroidMultiApp
    input_files = subapp.i
  []
[]

[Transfers]
  [from_primary_to_sub_pp]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    to_multi_app = sub
    source_variable = to_subapp
    postprocessor = from_primary_pp
  []
  [primary_average]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    from_multi_app = sub
    source_variable = from_subapp
    postprocessor = to_primary_pp
  []
  [array_var]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    from_multi_app = sub
    source_variable = array_var
    source_variable_component = 2
    postprocessor = to_primary_pp
  []
[]
