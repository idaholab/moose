[Mesh]
  file = rectangle.e
  boundary_name = 'left right'
  boundary_id = '1 2'
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  active = 'diff body_force'

  [./diff]
    type = DiffMKernel
    variable = u
    mat_prop = diff1
  [../]

  [./body_force]
    type = BodyForce
    variable = u
    block = 1
    value = 10
  [../]
[]

[BCs]
  active = 'right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

# Setup an AuxBC to consume a material property on the boundary
[AuxBCs]
  [./aux_bc]
    type = MaterialRealAux
    variable = aux
    property = 'diff2'
    boundary = 'left'
  [../]
[]

[Materials]
  [./mat1]
    type = Diff1Material
    block = '1 2'
  [../]
  [./bnd_mat]
    type = Diff1Material
    boundary = 'left'
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


