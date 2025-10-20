# Simple coupled Cahn-Hilliard with diffusion
#
# Energy functional:
# F[c,T] = ∫ [f_ch(c,∇c) + f_thermal(T,∇T) + coupling*c*T] dx
# where:
#   f_ch = (c^2-1)^2 + κ/2|∇c|^2
#   f_thermal = k/2|∇T|^2

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
  []
  [T]
    order = FIRST
    family = LAGRANGE
  []
[]

[AutomaticWeakForm]
  energy_type = expression

  # Combined energy functional for coupled Cahn-Hilliard and thermal diffusion
  # Chemical energy: W(c) + κ/2|∇c|^2
  # Thermal energy: k_th/2|∇T|^2
  # Coupling: β*c*T
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*k_th*dot(grad(T), grad(T)) + beta*c*T'

  parameters = 'kappa 0.01 k_th 1.0 beta 0.1'
  variables = 'c T'
  # Need to specify which variables are coupled for the kernel generation
  coupled_variables = 'c T'

  use_automatic_differentiation = true
[]

[ICs]
  [c_IC]
    type = RandomIC
    variable = c
    min = -0.05
    max = 0.05
    seed = 12345
  []
  [T_IC]
    type = ConstantIC
    variable = T
    value = 1.0
  []
[]

[BCs]
  # Temperature boundary conditions
  [T_left]
    type = DirichletBC
    variable = T
    boundary = left
    value = 1.0
  []
  [T_right]
    type = DirichletBC
    variable = T
    boundary = right
    value = 0.5
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_tol = 1e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  dt = 1e-3
  end_time = 0.01
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]