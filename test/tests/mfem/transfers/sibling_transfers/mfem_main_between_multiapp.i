# Base input for testing between-multiapp transfers. It has the following complexities:
# - multiapps may not be run with the same number of ranks
# - both nodal and elemental variables
# - transfers between mixes of nodal and elemental variables
# Tests derived from this input may add or remove complexities through command line arguments

[Problem]
  type = MFEMProblem
  solve = false
[]

[Mesh]
  type = MFEMMesh
  file = ../../mesh/square_quad.e
[]

[MultiApps]
  [mfem_send]
    type = TransientMultiApp
    input_files = mfem_sub_between_diffusion.i
  []
  [mfem_recv]
    type = TransientMultiApp
    input_files = mfem_sub_between_diffusion.i
    cli_args = Outputs/inactive=''
  []
[]

[Transfers]
  # Nodal to nodal variables
  [mfem_to_mfem_nodal_nodal]
    type = MultiAppMFEMShapeEvaluationTransfer
    from_multi_app = mfem_send
    to_multi_app = mfem_recv
    source_variables = sent_nodal
    variables = received_nodal
  []
[]

[Executioner]
  type = MFEMTransient
  num_steps = 2
[]

# Positions used to create multiple subapps for the 1 to N tests
[Positions]
  [mfem_send_locs]
    type = InputPositions
    positions = '0 0 0'
  []
  [mfem_recv_locs]
    type = InputPositions
    positions = '0 0 0
                 0.1 0.4 0
                 0.7 0.2 0'
  []
[]

[Outputs]
  file_base = OutputData/MFEMtoMFEMSiblingTransfer
[]
