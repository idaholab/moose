[Mesh]
  [file]
    type = FileMeshGenerator
    file = square.e
  []

  # Mesh Modifiers
  # If no dependencies are defined, the order of execution is not defined (based on pointer locations) so
  # this test case has several dependencies to minimize the chance of getting lucky when things aren't defined properly.
  # Rotations along different axes must occur in a defined order to end up at the right orientation at the end.
  # The final mesh will be angled at 45 degrees with new sidesets where there were none before.

  [add_side_sets]
    type = SideSetsFromNormalsGenerator
    input = last_rotate
    normals = ' 0.70710678118  0.70710678118  0
               -0.70710678118 -0.70710678118  0'
    new_boundary = 'up_right down_left'
    variance = 1e-3
    fixed_normal = true

  []

  [last_rotate]
    type = TransformGenerator
    input = rotate4
    transform = ROTATE
    vector_value = '-45 0 0'
  []

  [rotate1]
    type = TransformGenerator
    input = file
    transform = ROTATE
    vector_value = '0 0 82'
  []

  [rotate3]
    type = TransformGenerator
    input = rotate2
    transform = ROTATE
    vector_value = '0 36 0'
  []

  [rotate4]
    type = TransformGenerator
    input = rotate3
    transform = ROTATE
    vector_value = '0 0 -82'
  []

  [rotate2]
    type = TransformGenerator
    input = rotate1
    transform = ROTATE
    vector_value = '0 -36 0'
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
    boundary = down_left
    value = 0
  []
  [top]
    type = DirichletBC
    variable = u
    boundary = up_right
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
