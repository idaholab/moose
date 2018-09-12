# Using different mesh file where 1D elements will come before 2D ones.
# This is important for testing the robustness of re-initializing materials
[Mesh]
  file = 1d_2d-2.e
  # Mixed-dimension meshes don't seem to work with DistributedMesh.  The
  # program hangs, I can't get a useful stack trace when I attach to
  # it.  See also #2130.
  parallel_type = replicated
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusionTest
    variable = u
    prop_name = matp
  [../]
[]

[Materials]
  [./mat1]
    type = MTMaterial
    block = '1 2'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]

  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 100
    value = 0
  [../]

  [./top]
    type = DirichletBC
    variable = u
    boundary = 101
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
