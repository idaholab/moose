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
    [./box]
      type = BoxMarker
      bottom_left = '0.3 0.3 0'
      top_right = '0.6 0.6 0'
      inside = refine
      outside = coarsen
    [../]
    [./combo]
      type = ComboMarker
      markers = 'box box2'
    [../]
    [./box2]
      type = BoxMarker
      bottom_left = '0.5 0.5 0'
      top_right = '0.8 0.8 0'
      inside = dont_mark
      outside = refine
    [../]
  [../]
[]

[Outputs]
  exodus = true
[]
