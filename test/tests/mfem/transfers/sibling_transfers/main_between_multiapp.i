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

[MultiApps]
  [mfem_sub]
    type = TransientMultiApp
    input_files = mfem_sub_between_diffusion.i
  []
  [libmesh_sub]
    type = TransientMultiApp
    input_files = libmesh_sub_between_diffusion.i
  []
[]

[Transfers]
  # Nodal to nodal variables
  [mfem_to_libmesh_nodal_nodal]
    type = MultiAppMFEMTolibMeshShapeEvaluationTransfer
    from_multi_app = mfem_sub
    to_multi_app = libmesh_sub
    source_variables = sent_nodal
    variables = received_nodal
  []
  [libmesh_to_mfem_nodal_nodal]
    type = MultiApplibMeshToMFEMShapeEvaluationTransfer
    from_multi_app = libmesh_sub
    to_multi_app = mfem_sub
    source_variables = sent_nodal
    variables = received_nodal
  []

  # Elemental to elemental variables
  [mfem_to_libmesh_elem_elem]
    type = MultiAppMFEMTolibMeshShapeEvaluationTransfer
    from_multi_app = mfem_sub
    to_multi_app = libmesh_sub
    source_variables = sent_elem
    variables = received_elem
  []
  [libmesh_to_mfem_elem_elem]
    type = MultiApplibMeshToMFEMShapeEvaluationTransfer
    from_multi_app = libmesh_sub
    to_multi_app = mfem_sub
    source_variables = sent_elem
    variables = received_elem
  []

  # Elemental to nodal variables
  [mfem_to_libmesh_elem_nodal]
    type = MultiAppMFEMTolibMeshShapeEvaluationTransfer
    from_multi_app = mfem_sub
    to_multi_app = libmesh_sub
    source_variables = sent_elem
    variables = received_nodal
  []
  [libmesh_to_mfem_elem_nodal]
    type = MultiApplibMeshToMFEMShapeEvaluationTransfer
    from_multi_app = libmesh_sub
    to_multi_app = mfem_sub
    source_variables = sent_elem
    variables = received_nodal
  []

  # Nodal to elemental variables
  [mfem_to_libmesh_nodal_elem]
    type = MultiAppMFEMTolibMeshShapeEvaluationTransfer
    from_multi_app = mfem_sub
    to_multi_app = libmesh_sub
    source_variables = sent_nodal
    variables = received_elem
  []
  [libmesh_to_mfem_nodal_elem]
    type = MultiApplibMeshToMFEMShapeEvaluationTransfer
    from_multi_app = libmesh_sub
    to_multi_app = mfem_sub
    source_variables = sent_nodal
    variables = received_elem
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

# Positions used to create multiple subapps for the 1 to N and N to 1 tests
[Positions]
  [mfem_locs]
    type = InputPositions
    positions = '0 0 0
                 0.2 0.5 0'
  []
  [libmesh_locs]
    type = InputPositions
    positions = '0 0 0
                 0.1 0.4 0
                 0.7 0.2 0'
  []
[]
