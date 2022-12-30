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
    initial_condition = '-1 -1'
    components = 2
  []
  [from_sub_elem]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = '-1 -1'
    components = 2
  []
  [to_sub]
    components = 2
    [InitialCondition]
      type = ArrayFunctionIC
      function = '1+2*x*x+3*y*y*y 1.5+2*x*x+3*y*y*y'
    []
  []
  [to_sub_elem]
    components = 2
    order = CONSTANT
    family = MONOMIAL
    [InitialCondition]
      type = ArrayFunctionIC
      function = '2+2*x*x+3*y*y*y 3+2*x*x+3*y*y*y'
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
    # The offsets are to avoid equidistant points
    positions = '0.000001 0 0 0.4111 0.4112 0 0.6999 0.099 0'
    type = FullSolveMultiApp
    app_type = MooseTestApp
    input_files = sub_array.i
    output_in_position = true
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppGeneralFieldNearestNodeTransfer
    to_multi_app = sub
    source_variable = 'to_sub to_sub'
    source_variable_components = '1 0'
    variable = 'from_main from_main'
    target_variable_components = '0 1'
  []

  [to_sub_elem]
    type = MultiAppGeneralFieldNearestNodeTransfer
    to_multi_app = sub
    source_variable = 'to_sub_elem to_sub_elem'
    source_variable_components = '1 0'
    variable = 'from_main_elem from_main_elem'
    target_variable_components = '0 1'
  []

  [from_sub]
    type = MultiAppGeneralFieldNearestNodeTransfer
    from_multi_app = sub
    source_variable = 'to_main to_main'
    source_variable_components = '1 0'
    variable = 'from_sub from_sub'
    target_variable_components = '0 1'
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldNearestNodeTransfer
    from_multi_app = sub
    source_variable = 'to_main_elem to_main_elem'
    source_variable_components = '1 0'
    variable = 'from_sub_elem from_sub_elem'
    target_variable_components = '0 1'
  []
[]
