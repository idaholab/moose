[Mesh]
  type = FileMesh
  file = square.e
  # This MeshModifier currently only works with ReplicatedMesh.
  # For more information, refer to #2129.
  parallel_type = replicated
[]

# Mesh Modifiers
[MeshModifiers]
  # If no dependencies are defined, the order of execution is not defined (based on pointer locations) so
  # this test case has several dependencies to minimize the chance of getting lucky when things aren't defined properly.
  # Rotations along different axes must occur in a defined order to end up at the right orientation at the end.
  # The final mesh will be angled at 45 degrees with new sidesets where there were none before.

  [./add_side_sets]
    type = SideSetsFromNormals
    normals = ' 0.70710678118  0.70710678118  0
               -0.70710678118 -0.70710678118  0'
    new_boundary = 'up_right down_left'
    variance = 1e-3
    fixed_normal = true

    depends_on = last_rotate
  [../]

  [./last_rotate]
    type = Transform
    transform = ROTATE
    vector_value = '-45 0 0'

    depends_on = rotate4
  [../]

  [./rotate1]
    type = Transform
    transform = ROTATE
    vector_value = '0 0 82'
  [../]

  [./rotate3]
    type = Transform
    transform = ROTATE
    vector_value = '0 36 0'

    depends_on = rotate2
  [../]

  [./rotate4]
    type = Transform
    transform = ROTATE
    vector_value = '0 0 -82'

    depends_on = rotate3
  [../]

  [./rotate2]
    type = Transform
    transform = ROTATE
    vector_value = '0 -36 0'

    depends_on = rotate1
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = down_left
    value = 0
  [../]
  [./top]
    type = DirichletBC
    variable = u
    boundary = up_right
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
