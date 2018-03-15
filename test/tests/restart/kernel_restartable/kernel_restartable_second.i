###########################################################
# This test exercises the restart system and verifies
# correctness with parallel computation, but distributed
# and with threading.
#
# See kernel_restartable.i
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

[Kernels]
  [./diff]
    type = RestartDiffusion
    variable = u
    coef = 1
  [../]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
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
[]

[Problem]
  restart_file_base = kernel_restartable_restart_cp/LATEST
[]
