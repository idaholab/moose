[Mesh]
  type = FileMesh
  file = cylinder.e
  # This MeshModifier currently only works with SerialMesh.
  # For more information, refer to #2129.
  distribution = serial
[]

# Mesh Modifiers
[MeshModifiers]
  [./add_side_sets]
    type = SideSetsFromNormals
    normals = '0  1  0
               0 -1  0'
    boundary = 'front back'

    # This parameter makes it so that we won't
    # adjust the normal for each adjacent element.
    fixed_normal = true

    variance = 0.5
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
  [./front]
    type = DirichletBC
    variable = u
    boundary = front
    value = 0
  [../]
  [./back]
    type = DirichletBC
    variable = u
    boundary = back
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  print_linear_residuals = true

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]


