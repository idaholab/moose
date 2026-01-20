[Mesh]
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 3
    nz = 4
  []
  [combine_3_sides]
    type = RenameBoundaryGenerator
    input = 'cube'
    old_boundary = 'top front right'
    new_boundary = 'corner corner corner'
  []
  [project_onto_plane]
    type = ProjectSideSetOntoLevelSetGenerator
    input = 'combine_3_sides'
    direction = '1 1 1'
    sideset = 'corner'
    level_set = 'x + y + z - 10'
  []
[]
