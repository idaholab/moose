[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
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

  solve_type = 'PJFNK'
[]

[Adaptivity]
  [./Markers]
    [./marker]
      type = ValueThresholdMarker
      coarsen = 0.3
      variable = u
      refine = 0.7
    [../]
    [./inverted_marker]
      type = ValueThresholdMarker
      invert = true
      coarsen = 0.7
      refine = 0.3
      variable = u
      third_state = DO_NOTHING
    [../]
  [../]
[]

[Outputs]
  exodus = true
[]
