[Mesh]
  # Note: don't change the parameters without also changing extrude_square,
  # as they should be using identical file(s) in gold
  [square]
    type = GeneratedMeshGenerator
    dim = 2
  []
  [lowerDblock]
    type = LowerDBlockFromSidesetGenerator
    input = square
    new_block_name = "extrusions0"
    sidesets = "right"
  []
  [separateMesh]
    type = BlockToMeshConverterGenerator
    input = lowerDblock
    target_blocks = extrusions0
  []
  [extrude]
    type = MeshExtruderGenerator
    input = separateMesh
    num_layers = 3
    extrusion_vector = '1 0.5 0'
    bottom_sideset = 'new_bottom'
    top_sideset = 'new_top'
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'square extrude'
    stitch_boundaries_pairs = 'right new_bottom'
  []
[]
