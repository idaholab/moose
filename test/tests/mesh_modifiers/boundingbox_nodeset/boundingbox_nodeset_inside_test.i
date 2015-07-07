[Mesh]
  # This MeshModifier currently only works with SerialMesh.
  # For more information, refer to #2129.
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  distribution = serial
[]

[MeshModifiers]
  [./middle_node]
    type = BoundingBoxNodeSet
    new_boundary = middle_node
    top_right = '1 1 0'
    bottom_left = '0.5 0.5 0'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
  [./middle]
    type = DirichletBC
    variable = u
    boundary = middle_node
    value = -1
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  file_base = boundingbox_nodeset_inside_out
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]

