[Mesh]
  [eg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  # Produce an h-refined mesh that still carries its refinement tree
  [refine]
    type = RefineBlockGenerator
    input = eg
    block = '0'
    refinement = '1'
  []
  # Flatten the refinement tree back into ordinary level-0 elements
  [flatten]
    type = FlattenRefinementGenerator
    input = refine
  []
[]
