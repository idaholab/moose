# Base input for testing between-multiapp transfers. It has the following complexities:
# - multiapps may not be run with the same number of ranks
# - both nodal and elemental variables
# - transfers between mixes of nodal and elemental variables
# Tests derived from this input may add or remove complexities through command line arguments

[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

# This application use at most 3 processes
[MultiApps]
  [ma1]
    type = TransientMultiApp
    input_files = sub_between_diffusion1.i
    max_procs_per_app = 3
    output_in_position = true
  []
[]

# This application will use as many processes as the main app
[MultiApps]
  [ma2]
    type = TransientMultiApp
    input_files = sub_between_diffusion2.i
    output_in_position = true
  []
[]

[Transfers]
  # Nodal to nodal variables
  [app1_to_2_nodal_nodal]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = ma1
    to_multi_app = ma2
    source_variable = sent_nodal
    variable = received_nodal
    extrapolation_constant = -1
    # Test features non-overlapping meshes
    error_on_miss = false
  []
  [app2_to_1_nodal_nodal]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = ma2
    to_multi_app = ma1
    source_variable = sent_nodal
    variable = received_nodal
    extrapolation_constant = -1
    # Test features non-overlapping meshes
    error_on_miss = false
  []

  # Elemental to elemental variables
  [app1_to_2_elem_elem]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = ma1
    to_multi_app = ma2
    source_variable = sent_elem
    variable = received_elem
    extrapolation_constant = -1
    # Test features non-overlapping meshes
    error_on_miss = false
  []
  [app2_to_1_elem_elem]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = ma2
    to_multi_app = ma1
    source_variable = sent_elem
    variable = received_elem
    extrapolation_constant = -1
    # Test features non-overlapping meshes
    error_on_miss = false
  []

  # Elemental to nodal variables
  [app1_to_2_elem_nodal]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = ma1
    to_multi_app = ma2
    source_variable = sent_elem
    variable = received_nodal
    extrapolation_constant = -1
    # Test features non-overlapping meshes
    error_on_miss = false
  []
  [app2_to_1_elem_nodal]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = ma2
    to_multi_app = ma1
    source_variable = sent_elem
    variable = received_nodal
    extrapolation_constant = -1
    # Test features non-overlapping meshes
    error_on_miss = false
  []

  # Nodal to elemental variables
  [app1_to_2_nodal_elem]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = ma1
    to_multi_app = ma2
    source_variable = sent_nodal
    variable = received_elem
    extrapolation_constant = -1
    # Test features non-overlapping meshes
    error_on_miss = false
  []
  [app2_to_1_nodal_elem]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = ma2
    to_multi_app = ma1
    source_variable = sent_nodal
    variable = received_elem
    extrapolation_constant = -1
    # Test features non-overlapping meshes
    error_on_miss = false
  []
[]

# To create multiple subapps for the 1 to N and N to M tests
# The subapps should not overlap for shape evaluation transfers
# or at least, the block restriction of the source variables between
# applications should not overlap
[Positions]
  [app1_locs]
    type = InputPositions
    positions = '0 0 0
                 0 1.01 0'
  []
  # Keep in mind app2's mesh is offset
  [app2_locs]
    type = InputPositions
    positions = '-0.7 -0.45 0
                 0.7 0.3 0
                 -0.5 0.5 0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
