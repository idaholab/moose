[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 0
  zmin = 0
  zmax = 0
  elem_type = QUAD4
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
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Adaptivity]
  [./Markers]
    [./test]
      type = StandardMarker
      field_name = marker
      indicator_field = dummy
    [../]
  [../]
[]

[Output]
  linear_residuals = true
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]

[Debug]
  show_actions = true
[]

