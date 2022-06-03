# ##########################################################
# This is a test of the Multiapp System. This test solves
# four independent applications spaced throughout a
# parent domain interleaved with a parent solve.
#
# @Requirement F7.10
# ##########################################################
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
    type = Diffusion
    variable = u
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
    boundary = 'left'
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./sub_app]
    type = TransientMultiApp
    positions = '0 0 0  0.5 0.5 0'
    input_files = 'sub.i failing_sub.i'
    app_type = MooseTestApp
    execute_on = 'timestep_end'
    max_catch_up_steps = 100
    max_failures = 100
    catch_up = true
  [../]
[]
