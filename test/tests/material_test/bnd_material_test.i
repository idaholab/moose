[Mesh]
  type = GeneratedMesh
  dim = 3
	xmin = 0
	xmax = 1
	ymin = 0
	ymax = 1
	zmin = 0
	zmax = 1
	nx = 3
	ny = 3
	nz = 3
[]

# Nonlinear system

[Variables]
  active = 'u'

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
    boundary = left
    value = 1
  [../]

  [./right]
    type = MTBC
    variable = u
    boundary = right
    grad = 8
    prop_name = matp
  [../]
[]

[Materials]
  [./mat_left]
    type = MTMaterial
    boundary = left
  [../]
  [./mat_right]
    type = MTMaterial
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  output_initial = true
  elemental_as_nodal = true
  interval = 1
  exodus = true
  perf_log = true
[]
