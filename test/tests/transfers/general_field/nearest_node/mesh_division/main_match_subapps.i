# Base input for testing transfers with mesh divisions restrictions. The mesh divisions
# in the parent app will be matched with a subapp index.
# In the to_multiapp direction, the main app data at the mesh division bins of index 1-4 will
# be transferred to subapps of index 1-4 respectively
# In the from_multiapp direction, the main app fields at the mesh divisions bins of index 1-4
# will receive data (be transferred) from subapps of index 1-4 respectively
# It has the following complexities:
# - several sub-applications
# - transfers both from and to the subapps
# - both nodal and elemental variables
# Tests derived from this input may add complexities through command line arguments

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[MeshDivisions]
  [middle]
    type = CartesianGridDivision
    bottom_left = '0.21 0.21 0'
    # cover more and sample more bins
    top_right = '1.001 1.001 0'
    nx = 2
    ny = 2
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
    # The positions are randomly offset to prevent equi-distant nearest-locations
    positions = '0.1001 0.0000013 0
                 0.30054 0.600001985 0
                 0.70021 0.4000022 0
                 0.800212 0.8500022 0'
    # To differentiate the values received from each subapp
    cli_args = 'base_value=1 base_value=2 base_value=3 base_value=4'
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    source_variable = to_sub
    variable = from_main
    from_mesh_division = middle
    from_mesh_division_usage = 'matching_subapp_index'
  []

  [to_sub_elem]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    source_variable = to_sub_elem
    variable = from_main_elem
    from_mesh_division = middle
    from_mesh_division_usage = 'matching_subapp_index'
  []

  [from_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = to_main
    variable = from_sub
    to_mesh_division = middle
    to_mesh_division_usage = 'matching_subapp_index'
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = to_main_elem
    variable = from_sub_elem
    to_mesh_division = middle
    to_mesh_division_usage = 'matching_subapp_index'
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
