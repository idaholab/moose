###########################################################
# This test exercises the parallel computation aspect of
# the framework. Seperate input mesh files are read on
# different processors and separate output files are
# produced on different processors.
#
# @Requirement P1.10
###########################################################


[Mesh]
  file = cylinder/cylinder.e
  nemesis = true
  # This option lets us exodiff against a gold standard generated
  # without repartitioning
  skip_partitioning = true
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right'

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

[Postprocessors]
  [./elem_avg]
    type = ElementAverageValue
    variable = u
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
  nemesis = true
[]
