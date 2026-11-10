!include grain_boundary_sczm_coarse_lambda1.i

[Mesh]
  [refined_boundary]
    type = RefineBlockGenerator
    input = boundary_mesh
    block = grain1
    refinement = 1
    enable_neighbor_refinement = false
    save_with_name = refined_boundary
  []
[]

[Physics/SolidMechanics/ShiftedCohesiveZone/czm_ik]
  complete_interface_mesh := refined_boundary
[]

[Materials/interface_traction]
  boundary := grain1_grain2
[]
