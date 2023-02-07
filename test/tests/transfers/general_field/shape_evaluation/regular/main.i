# Base input for testing transfers. It has the following complexities:
# - more than one subapp
# - transfers both from and to the subapps
# - both nodal and elemental variables
# Tests derived from this input may add complexities through command line arguments

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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
    hide = 'to_sub to_sub_elem'
    overwrite = true
  []
[]

[MultiApps]
  [sub]
    # 1 on corner, one in the center and one close to a corner
    positions = '0 0 0 0.4 0.4 0 0.7 0.1 0'
    type = FullSolveMultiApp
    app_type = MooseTestApp
    input_files = sub.i
    output_in_position = true
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = sub
    source_variable = to_sub
    variable = from_main
    extrapolation_constant = -1
  []

  [to_sub_elem]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = sub
    source_variable = to_sub_elem
    variable = from_main_elem
    extrapolation_constant = -1
  []

  [from_sub]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = sub
    source_variable = to_main
    variable = from_sub
    extrapolation_constant = -1
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = sub
    source_variable = to_main_elem
    variable = from_sub_elem
    extrapolation_constant = -1
  []
[]
