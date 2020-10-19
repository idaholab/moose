[Mesh]
  [file]
    type = FileMeshGenerator
    file = cylinder.e
  []

  # Mesh Modifiers
  [add_side_sets]
    type = SideSetsFromNormalsGenerator
    input = file
    normals = '0  0  1
               0  1  0
               0  0 -1'

    # This parameter allows the normal
    # to vary slightly from adjacent element
    # to element so that a sidset can follow
    # a curve. It is false by default.
    fixed_normal = false
    new_boundary = 'top side bottom'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
