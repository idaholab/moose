# This test makes sure we check both nonlinear systems for steady state detection
# by having the second system be a lot slower to develop a steady state

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[Problem]
  nl_sys_names = 'nl0 nl1'
[]

[Variables]
  [u]
    solver_sys = 'nl0'
  []
  [v]
    solver_sys = 'nl1'
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [time]
    type = TimeDerivative
    variable = u
  []
  [diff_slower]
    type = CoefDiffusion
    variable = v
    coef = 0.001
  []
  [time_v]
    type = TimeDerivative
    variable = v
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [leftv]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [rightv]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  steady_state_detection = true
  steady_state_tolerance = 1e-2
  nl_abs_tol = 1e-10

  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
  []
[]

[Outputs]
  csv = true
[]
