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

  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [aux_elem]
    order = CONSTANT
    family = MONOMIAL
  []

  [aux_nodal]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [aux_elem]
    type = CoupledAux
    variable = aux_elem
    operator = '*'
    value = 1
    coupled = u
  []

  [aux_nodal]
    type = CoupledAux
    variable = aux_nodal
    operator = '*'
    value = 1
    coupled = u
  []
[]

[Kernels]
  active = 'diff'

  [diff]
    type = Diffusion
    variable = u
    block = block_1
  []
[]

[BCs]
  active = 'left right'

  [left]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = top
    value = 1
  []
[]

[Postprocessors]
  [elem_avg]
    type = ElementAverageValue
    variable = u
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  file_base = out
  nemesis = true
[]
