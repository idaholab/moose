[Mesh]
  type = PatternedMesh
  files = 'quad_mesh.e tri_mesh.e'
  pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
             0 1 1 0 0 0 0 0 0 0 0 1 1 0 ;
             0 1 1 1 0 0 0 0 0 0 1 1 1 0 ;
             0 1 0 1 1 0 0 0 0 1 1 0 1 0 ;
             0 1 0 0 1 1 0 0 1 1 0 0 1 0 ;
             0 1 0 0 0 1 1 1 1 0 0 0 1 0 ;
             0 1 0 0 0 0 1 1 0 0 0 0 1 0 ;
             0 1 0 0 0 0 0 0 0 0 0 0 1 0 ;
             0 1 0 0 0 0 0 0 0 0 0 0 1 0 ;
             0 1 0 0 0 0 0 0 0 0 0 0 1 0 ;
             0 1 0 0 0 0 0 0 0 0 0 0 1 0 ;
             0 0 0 0 0 0 0 0 0 0 0 0 0 0'
  bottom_boundary = 1
  right_boundary = 2
  top_boundary = 3
  left_boundary = 4
  y_width = 1.2
  x_width = 1.2
  parallel_type = replicated
[]
