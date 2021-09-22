[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  elem_type = QUAD4
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Reporters]
  [coords]
    type=ConstantReporter
    real_vector_names = 'x y z'
    real_vector_values = '.31 .41 .51  .31 .41 .51 .31 .41 .51 .8;
                          .31 .31 .31 .41  .41 .41 .51 .51 .51 .8;
                           0   0   0  0   0   0  0  0   0  1;'
    outputs=none
  []
  [bad_coords]
    type=ConstantReporter
    real_vector_names = 'x y z'
    real_vector_values = '.31 .41 .51;
                          .31 .31 .31 .41  .41 .41 .51 .51;
                           0   0   0  0   0   0  0  0   0  1;'
    outputs=none
  []
[]

[Adaptivity]
  [Markers]
      active = 'box'
    [box]
      type = ReporterPointMarker
      x_coord_name = coords/x
      y_coord_name = coords/y
      z_coord_name = coords/z
      inside = refine
      empty = do_nothing
    []
    [bad_coord]
      type = ReporterPointMarker
      x_coord_name = bad_coords/x
      y_coord_name = bad_coords/y
      z_coord_name = bad_coords/z
      inside = refine
      empty = do_nothing
    []
  []
[]

[Outputs]
  exodus=true
[]
