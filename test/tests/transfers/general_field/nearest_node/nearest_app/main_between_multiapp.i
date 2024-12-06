# Base input for testing between-multiapp transfers. It has the following complexities:
# - multiapps may not be run with the same number of ranks
# - both nodal and elemental variables
# Tests derived from this input may add or remove complexities through command line arguments

[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Positions]
  [input_app1]
    type = InputPositions
    positions = '0 0.1 0
                 0.5 0.5 0'
  []
  [input_app2]
    type = InputPositions
    # offsets to avoid indetermination
    # but small enough to remain below to bounding box factor bump
    positions = '0.0000001 0.30000000001 0
                 0.60000000001 0.5003 0'
  []
[]

# This application uses at most 3 processes
[MultiApps/ma1]
  type = TransientMultiApp
  input_files = sub_between_diffusion.i
  max_procs_per_app = 3
  positions_objects = 'input_app1'
  output_in_position = true
  # We must ensure that each point on each app is closest to the app
  # position than to any other app position.
  # This is to ensure that nearest-app is the same as nearest-position with the app position
  cli_args = "Mesh/gen/xmax=0.1;Mesh/gen/ymax=0.1"
[]

# This application will use as many processes as the main app
[MultiApps/ma2]
  type = TransientMultiApp
  input_files = sub_between_diffusion.i
  positions_objects = 'input_app2'
  output_in_position = true
  cli_args = "Mesh/gen/xmax=0.1;Mesh/gen/ymax=0.1"
[]

# slight inflation to avoid floating point issues on borders
bbox_factor_tr = 1.0001


[Transfers]
  # Nodal to nodal variables
  [app1_to_2_nodal_nodal]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = ma1
    to_multi_app = ma2
    source_variable = sent_nodal
    variable = received_nodal
    assume_nearest_app_holds_nearest_location = true
    search_value_conflicts = true
    bbox_factor = ${bbox_factor_tr}
  []
  [app2_to_1_nodal_nodal]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = ma2
    to_multi_app = ma1
    source_variable = sent_nodal
    variable = received_nodal
    assume_nearest_app_holds_nearest_location = true
    search_value_conflicts = true
    bbox_factor = ${bbox_factor_tr}
  []

  # Elemental to elemental variables
  [app1_to_2_elem_elem]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = ma1
    to_multi_app = ma2
    source_variable = sent_elem
    variable = received_elem
    assume_nearest_app_holds_nearest_location = true
    search_value_conflicts = true
    bbox_factor = ${bbox_factor_tr}
  []
  [app2_to_1_elem_elem]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = ma2
    to_multi_app = ma1
    source_variable = sent_elem
    variable = received_elem
    assume_nearest_app_holds_nearest_location = true
    search_value_conflicts = true
    bbox_factor = ${bbox_factor_tr}
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
