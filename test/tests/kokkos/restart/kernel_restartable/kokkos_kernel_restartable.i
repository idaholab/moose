###########################################################
# This test exercises the restart system and verifies
# correctness with parallel computation, but distributed
# and with threading.
#
# See kernel_restartable_second.i
#
# @Requirement F1.60
# @Requirement P1.10
# @Requirement P1.20
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[KokkosKernels]
  [./diff]
    type = KokkosRestartDiffusion
    variable = u
  [../]
  [./td]
    type = KokkosTimeDerivative
    variable = u
  [../]
[]

[KokkosBCs]
  [./left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = KokkosDirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1e-2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  [./restart]
    type = Checkpoint
    num_files = 100
  [../]
[]
