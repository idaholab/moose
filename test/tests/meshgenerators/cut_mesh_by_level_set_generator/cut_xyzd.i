[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 10
    nz = 10
    xmin = -1
    ymin = -1
    zmin = -1
    elem_type = HEX8
  []
  [lsc]
    type = CutMeshByLevelSetGenerator
    input = gmg
    level_set = 'x*x+y*y+z*z-0.81'
    cut_face_id = 345
    cut_face_name =ls
  []
  [xyzd]
    type = XYZDelaunayGenerator
    boundary = lsc
    desired_volume = 1
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [volume]
    type = VolumePostprocessor
  []
[]

[Problem]
  solve = false
[]

[Outputs]
  csv = true
[]
