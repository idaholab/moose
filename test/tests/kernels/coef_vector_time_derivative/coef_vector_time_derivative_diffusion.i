# ─────────────────────────────────────────────────────────────────────────────
# Test for CoefVectorTimeDerivative kernel
# Solves: c * dA/dt = nabla^2 A with c = pi^2
# Manufactured solution: A_x = exp(-t) * sin(pi*x), A_y = 0
# ─────────────────────────────────────────────────────────────────────────────

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 1.0
  []
[]

[Variables]
  [A]
    family = LAGRANGE_VEC
    order = FIRST
  []
[]

[ICs]
  [A_ic]
    type = VectorFunctionIC
    variable = A
    function_x = 'sin(pi*x)'
    function_y = '0'
    function_z = '0'
  []
[]

[Kernels]
  [A_time]
    type = CoefVectorTimeDerivative
    variable = A
    coef = 9.8696      # pi^2
  []
  [A_diff]
    type = VectorDiffusion
    variable = A
  []
[]

[BCs]
  [A_zero]
    type = VectorFunctionDirichletBC
    variable = A
    boundary = 'left right'
    function_x = '0'
    function_y = '0'
    function_z = '0'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  start_time = 0.0
  end_time = 1.0
  dt = 0.1
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
