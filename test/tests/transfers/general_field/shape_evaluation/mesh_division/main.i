# Base input for testing transfers. It has the following complexities:
# - transfers both from and to the subapps
# - both nodal and elemental variables
# Tests derived from this input may add complexities through command line arguments

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[MeshDivisions]
  [middle]
    type = CartesianGridDivision
    bottom_left = '0.01 0.01 0'
    top_right = '0.81 0.81 0'
    nx = 4
    ny = 4
    nz = 1
  []
[]

[AuxVariables]
  [from_sub]
    initial_condition = -1
  []
  [from_sub_elem]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = -1
  []
  [to_sub]
    [InitialCondition]
      type = FunctionIC
      function = '1 + 2*x*x + 3*y*y*y'
    []
  []
  [to_sub_elem]
    order = CONSTANT
    family = MONOMIAL
    [InitialCondition]
      type = FunctionIC
      function = '2 + 2*x*x + 3*y*y*y'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  [out]
    type = Exodus
    hide = 'to_sub to_sub_elem div'
    overwrite = true
  []
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    input_files = sub.i
    output_in_position = true
    # we want to avoid sampling on a boundary
    positions = '0.00001 0.0001 0'
    cli_args = 'base_value=1'
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = sub
    source_variable = to_sub
    variable = from_main
    # Test features non-overlapping meshes
    error_on_miss = false
  []

  [to_sub_elem]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = sub
    source_variable = to_sub_elem
    variable = from_main_elem
    # Test features non-overlapping meshes
    error_on_miss = false
  []

  [from_sub]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = sub
    source_variable = to_main
    variable = from_sub
    # Test features non-overlapping meshes
    error_on_miss = false
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = sub
    source_variable = to_main_elem
    variable = from_sub_elem
    # Test features non-overlapping meshes
    error_on_miss = false
  []
[]

# For debugging purposes
[AuxVariables]
  [div]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [mesh_div]
    type = MeshDivisionAux
    variable = div
    mesh_division = 'middle'
  []
[]
